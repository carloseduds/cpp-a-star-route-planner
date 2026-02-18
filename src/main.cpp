#include <optional>
#include <fstream>
#include <iostream>
#include <limits>
#include <vector>
#include <string>
#include <string_view>
#include <cstdlib>

#include <io2d.h>

#include "route_model.h"
#include "render.h"
#include "route_planner.h"

using namespace std::experimental;

static std::optional<std::vector<std::byte>> ReadFile(const std::string &path)
{
    std::ifstream is{path, std::ios::binary | std::ios::ate};
    if (!is) return std::nullopt;

    const auto size = is.tellg();
    if (size <= 0) return std::nullopt;

    std::vector<std::byte> contents(static_cast<size_t>(size));
    is.seekg(0);
    is.read(reinterpret_cast<char*>(contents.data()), size);

    if (contents.empty()) return std::nullopt;
    return contents;
}

struct Args {
    std::string osm_file = "../map.osm";
    std::string out_png  = "map_routed.png";
    bool show_window = false;
    bool have_coords = false;
    float start_x = 20.f, start_y = 30.f, end_x = 50.f, end_y = 40.f;
};

static void PrintUsage(const char* exe) {
    std::cout
        << "Usage:\n"
        << "  " << exe << " [-f map.osm] [--out map_routed.png] [--show]\n"
        << "  " << exe << " [-f map.osm] [--out map_routed.png] --start x y --end x y [--show]\n\n"
        << "Notes:\n"
        << "  - Sem --start/--end, o programa pede as coordenadas via stdin (0..100).\n"
        << "  - Por padrão ele renderiza em modo headless e salva o PNG.\n"
        << "  - Use --show para abrir a janela interativa (requer ambiente gráfico).\n";
}

static bool ParseArgs(int argc, const char** argv, Args& a) {
    for (int i = 1; i < argc; ++i) {
        std::string_view s{argv[i]};
        if (s == "-h" || s == "--help") {
            PrintUsage(argv[0]);
            return false;
        }
        if (s == "-f" && i + 1 < argc) {
            a.osm_file = argv[++i];
            continue;
        }
        if (s == "--out" && i + 1 < argc) {
            a.out_png = argv[++i];
            continue;
        }
        if (s == "--show") {
            a.show_window = true;
            continue;
        }
        if (s == "--start" && i + 2 < argc) {
            a.start_x = std::stof(argv[++i]);
            a.start_y = std::stof(argv[++i]);
            a.have_coords = true;
            continue;
        }
        if (s == "--end" && i + 2 < argc) {
            a.end_x = std::stof(argv[++i]);
            a.end_y = std::stof(argv[++i]);
            a.have_coords = true;
            continue;
        }
        std::cerr << "Argumento desconhecido: " << s << "\n";
        PrintUsage(argv[0]);
        return false;
    }
    return true;
}

static float GetValidCoord(const std::string &prompt)
{
    float value = 0.0f;
    while (true) {
        std::cout << prompt;
        if ((std::cin >> value) && value >= 0.0f && value <= 100.0f) {
            return value;
        }
        std::cout << "Valor invalido. Digite um numero entre 0 e 100.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

int main(int argc, const char **argv)
{
    Args args{};
    if (!ParseArgs(argc, argv, args)) {
        return 1;
    }

    // Read OSM data
    std::cout << "Reading OpenStreetMap data from the following file: " << args.osm_file << std::endl;
    auto data = ReadFile(args.osm_file);
    if (!data) {
        std::cerr << "Failed to read OSM file: " << args.osm_file << std::endl;
        return 2;
    }

    if (!args.have_coords) {
        // Get user input in the range [0, 100].
        args.start_x = GetValidCoord("Start x (0-100): ");
        args.start_y = GetValidCoord("Start y (0-100): ");
        args.end_x   = GetValidCoord("End x (0-100): ");
        args.end_y   = GetValidCoord("End y (0-100): ");
    }

    // Build Model.
    RouteModel model{*data};

    // A* search
    RoutePlanner route_planner{model, args.start_x, args.start_y, args.end_x, args.end_y};
    route_planner.AStarSearch();
    std::cout << "Distance: " << route_planner.GetDistance() << " meters.\n";

    // Render results (headless -> PNG)
    Render render{model};

    const int width = 400;
    const int height = 400;

    io2d::image_surface img{ io2d::format::argb32, width, height };
    render.Display(img);

    try {
        img.save(args.out_png, io2d::image_file_format::png);
        std::cout << "Wrote: " << args.out_png << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Failed to write PNG (" << args.out_png << "): " << e.what() << "\n";
        return 3;
    }

    if (args.show_window) {
        // Optional interactive window (requires graphical environment)
        auto display = io2d::output_surface{
            width, height,
            io2d::format::argb32,
            io2d::scaling::none,
            io2d::refresh_style::fixed,
            30
        };

        display.size_change_callback([](io2d::output_surface& surface){
            surface.dimensions(surface.display_dimensions());
        });
        display.draw_callback([&](io2d::output_surface& surface){
            render.Display(surface);
        });

        display.begin_show();
    }

    return 0;
}
