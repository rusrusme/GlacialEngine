//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <format>
#include <iostream>
#include <optional>
#include <string_view>

#include "Token.hpp"
#include "glaze/Compiler.hpp"

namespace {
    void print_usage_and_exit(std::string_view binary_name) {
        std::cerr << std::format("Usage: {} <source.gscript> [options]\n\n", binary_name)
            << "Options:\n"
            << "  -c, --check             Extensive syntax check using g++ - WARNING: May take a VERY long time\n"
            << "  -o, --output <path>     Output directory for generated script header files\n"
            << "  -e, --engine <path>     Path to Glacial Engine\n"
            << "  --engine=<path>         Path to Glacial root directory\n"
            << "  -h, --help              Shows this help message\n";
        std::exit(0);
    }
}

int main(int argc, char* argv[]) {
    std::string_view binary_name = argc > 0 && argv[0] ? argv[0] : "glazec";

    if (argc < 2) {
        std::cerr << "glazec: no source file specified\n";
        print_usage_and_exit(binary_name);
    }

    glaze::CompilerOptions options;
    std::optional<std::filesystem::path> source_file;

    for (int i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            print_usage_and_exit(binary_name);
        }
        else if (arg == "-c" || arg == "--check") {
            options.extensive_syntax_check = true;
        }
        else if (arg == "-o" || arg == "--output") {
            if (i + 1 >= argc) {
                std::cerr << std::format("glazec error: missing path after '{}'\n", arg);
                return 1;
            }
            options.output_path = argv[++i];
        }
        else if (arg == "-e" || arg == "--engine") {
            if (i + 1 >= argc) {
                std::cerr << std::format("glazec error: missing path after '{}'\n", arg);
                return 1;
            }
            options.engine_path = argv[++i];
        }
        else if (arg.starts_with("--engine=")) {
            options.engine_path = arg.substr(std::string_view("--engine=").size());
        }
        else if (arg.starts_with('-')) {
            std::cerr << std::format("glazec error: unknown option '{}'\n", arg);
            print_usage_and_exit(binary_name);
        }
        else {
            if (source_file.has_value()) {
                std::cerr << std::format("glazec error: multiple input files specified ('{}' and '{}')\n",
                                         source_file->string(), arg);
                return 1;
            }
            source_file = std::filesystem::path(arg);
        }
    }

    if (!source_file.has_value()) {
        std::cerr << "glazec error: no source file specified\n";
        print_usage_and_exit(binary_name);
    }

    try {
        glaze::Compiler compiler(std::move(options));
        return compiler.compile_file(*source_file);
    }
    catch (const glaze::GlazeCompilerError& e) {
        std::cerr << std::format("glazec compiler error: {}\n", e.what());
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << std::format("glazec error: {}\n", e.what());
        return 1;
    }
}
