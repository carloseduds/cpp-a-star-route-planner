cmake_minimum_required(VERSION 3.16)

project(OSM_A_star_search LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

option(OSM_FETCH_IO2D "Fetch and build cpp-io2d (P0267 reference implementation) automatically" ON)

# ----------------------------
# Dependencies
# ----------------------------
include(FetchContent)

set(IO2D_TARGET "")  # always set, regardless of branch

if (OSM_FETCH_IO2D)
    # cpp-io2d reference implementation
    # Repo: https://github.com/cpp-io2d/P0267_RefImpl
    set(IO2D_WITHOUT_SAMPLES ON CACHE BOOL "" FORCE)
    set(IO2D_WITHOUT_TESTS   ON CACHE BOOL "" FORCE)

    FetchContent_Declare(
        io2d
        GIT_REPOSITORY https://github.com/cpp-io2d/P0267_RefImpl.git
        GIT_TAG        caa0ba0cb5a421a38bc26afaf3505bee206c44dd
    )
    FetchContent_MakeAvailable(io2d)

    # ---- io2d target resolution (handles different target names across revisions)
    if (TARGET io2d::io2d)
        set(IO2D_TARGET io2d::io2d)
    elseif (TARGET io2d)
        set(IO2D_TARGET io2d)
    elseif (TARGET P0267_RefImpl)
        set(IO2D_TARGET P0267_RefImpl)
    elseif (TARGET p0267_refimpl)
        set(IO2D_TARGET p0267_refimpl)
    else()
        get_property(_allTargets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
        message(FATAL_ERROR "io2d was fetched, but no known io2d target was created. Targets seen: ${_allTargets}")
    endif()
else()
    find_package(io2d REQUIRED)

    # When using find_package, prefer the canonical imported target.
    if (TARGET io2d::io2d)
        set(IO2D_TARGET io2d::io2d)
    elseif (TARGET io2d)
        set(IO2D_TARGET io2d)
    else()
        message(FATAL_ERROR "find_package(io2d) succeeded, but no io2d target was found (expected io2d::io2d or io2d).")
    endif()
endif()

# vendored deps
add_subdirectory(thirdparty/pugixml)

# ----------------------------
# Main target
# ----------------------------
file(GLOB project_SRCS CONFIGURE_DEPENDS src/*.cpp src/*.h)

add_executable(OSM_A_star_search ${project_SRCS})
target_include_directories(OSM_A_star_search PRIVATE thirdparty/pugixml/src)

target_link_libraries(OSM_A_star_search
    PRIVATE ${IO2D_TARGET}
    PUBLIC  pugixml
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(OSM_A_star_search PRIVATE pthread)
endif()

if (MSVC)
    target_compile_options(OSM_A_star_search PUBLIC /D_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING /wd4459)
endif()

# ----------------------------
# Tests
# ----------------------------
include(CTest)
enable_testing()

# googletest (vendored)
add_subdirectory(thirdparty/googletest)

add_library(route_planner OBJECT
    src/route_planner.cpp
    src/model.cpp
    src/route_model.cpp
)
target_include_directories(route_planner PRIVATE thirdparty/pugixml/src)

add_executable(unit_tests test/utest_rp_a_star_search.cpp)
target_link_libraries(unit_tests PRIVATE gtest_main route_planner pugixml)

add_test(NAME unit_tests COMMAND unit_tests)
