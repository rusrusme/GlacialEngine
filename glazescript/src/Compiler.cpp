//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "glaze/Compiler.hpp"
#include "GChecker.hpp"
#include "GLexer.hpp"
#include "GParser.hpp"
#include "GTranspiler.hpp"

namespace glaze {
    namespace {
        std::optional<std::string> get_env_var(const char* name) {
            #if defined(_MSC_VER)
            char* buffer = nullptr;
            size_t size = 0;
            if (_dupenv_s(&buffer, &size, name) == 0 && buffer != nullptr) {
                std::string result(buffer);
                std::free(buffer);
                return result;
            }
            return std::nullopt;
            #else
            const char* val = std::getenv(name);
            return (val != nullptr) ? std::optional<std::string>(val) : std::nullopt;
            #endif
        }
    }

    Compiler::Compiler(CompilerOptions options)
        : options_(std::move(options)) {}

    int Compiler::compile_file(const std::filesystem::path& file_path) {
        std::filesystem::path abs_path = std::filesystem::absolute(file_path).lexically_normal();
        std::string filename = abs_path.filename().string();

        std::string lower_filename = filename;
        std::ranges::transform(lower_filename, lower_filename.begin(),
                               [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        if (!lower_filename.ends_with(".gscript")) {
            throw GlazeCompilerError("file must have '.gscript' extension");
        }

        std::ifstream file_stream(abs_path);
        if (!file_stream) {
            std::cerr << std::format("glazec i/o error: could not open file '{}'\n", abs_path.string());
            return 1;
        }

        std::stringstream buffer;
        buffer << file_stream.rdbuf();
        return compile_source(buffer.str(), filename, options_.output_path);
    }

    int Compiler::compile_source(std::string_view source_code,
                                 const std::string& script_name,
                                 const std::filesystem::path& output_dir) {
        GLexer lexer;
        std::vector<Token> tokens = lexer.tokenize(source_code);

        GParser parser(script_name, std::move(tokens));
        Program script = parser.construct();

        GChecker checker;
        checker.check(script);

        GTranspiler transpiler(checker.symbol_table(), checker.resolved_symbols(), checker.expression_types());
        std::string cpp_code = transpiler.generate(script);

        std::string clean_name = script_name;
        if (clean_name.size() >= 8) {
            std::string suffix = clean_name.substr(clean_name.size() - 8);
            std::string suffix_lower = suffix;
            std::ranges::transform(suffix_lower, suffix_lower.begin(),
                                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if (suffix_lower == ".gscript") {
                clean_name.erase(clean_name.size() - 8);
            }
        }

        std::filesystem::create_directories(output_dir);
        std::filesystem::path transpiled_header = output_dir / (clean_name + ".hpp");

        std::ofstream out(transpiled_header);
        if (!out) {
            std::cerr << std::format("glazec i/o error: failed to write to '{}'\n", transpiled_header.string());
            return 1;
        }
        out << cpp_code;
        out.close();

        if (!generate_bundle(output_dir)) {
            return 1;
        }

        if (!options_.extensive_syntax_check) {
            std::cout << std::format("glazec: Generated {}\n", transpiled_header.string());
            return 0;
        }
        return run_syntax_check(transpiled_header);
    }

    bool Compiler::generate_bundle(const std::filesystem::path& output_dir) {
        std::vector<std::string> header_files;
        std::vector<std::string> module_names;

        for (const auto& entry : std::filesystem::directory_iterator(output_dir)) {
            if (!entry.is_regular_file()) continue;
            auto path = entry.path();
            if (path.extension() == ".hpp") {
                header_files.push_back(path.filename().string());

                std::string mod_name = path.stem().string();
                std::ranges::transform(mod_name, mod_name.begin(),
                                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                module_names.push_back(std::move(mod_name));
            }
        }

        std::ranges::sort(header_files);
        std::ranges::sort(module_names);

        std::filesystem::path bundle_path = output_dir / "GlazeScriptsBundle.cpp";
        std::ofstream bundle(bundle_path);
        if (!bundle) {
            std::cerr << std::format("glazec i/o error: failed to write bundle '{}'\n", bundle_path.string());
            return false;
        }

        bundle << "// Automatically generated by GlazeScript. Do not edit manually.\n\n";
        bundle << "#include \"project/scene/ecs/ComponentRegistry.hpp\"\n\n";
        bundle << "#include \"script/ScriptRegistry.hpp\"\n\n";

        for (const auto& header : header_files) {
            bundle << std::format("#include \"{}\"\n", header);
        }

        bundle << "\n"
            << "#if defined(_WIN32)\n"
            << "    #define GLAZE_EXPORT __declspec(dllexport)\n"
            << "#else\n"
            << "    #define GLAZE_EXPORT __attribute__((visibility(\"default\")))\n"
            << "#endif\n\n"
            << "extern \"C\" {\n"
            << "    GLAZE_EXPORT void glaze_init_scripts(\n"
            << "        engine::ScriptRegistry::ModuleList* host_modules,\n"
            << "        engine::ComponentRegistry::CopierMap* host_copiers) {\n"
            << "        engine::ScriptRegistry::set_host_modules(host_modules);\n"
            << "        engine::ComponentRegistry::set_host_copiers(host_copiers);\n\n";

        for (const auto& mod : module_names) {
            bundle << std::format("        engine::ScriptRegistry::add_script_module(script::register_components_{0}, script::register_systems_{0});\n", mod);
        }

        bundle << "    }\n"
            << "}\n";

        return true;
    }

    int Compiler::run_syntax_check(const std::filesystem::path& header_path) const {
        std::filesystem::path root_path = std::filesystem::absolute(options_.engine_path).lexically_normal();
        std::filesystem::path include_path = std::filesystem::exists(root_path / "engine/include")
                                                 ? root_path / "engine/include"
                                                 : root_path / "include";

        auto deps_path_opt = resolve_fetch_content_deps(root_path);
        if (!deps_path_opt.has_value()) {
            std::cerr << std::format("glazec: warning: CMake '_deps' folder not found under {}\n", root_path.string());
            std::cerr << "glazec: Skipping g++ syntax check.\n";
            std::cout << std::format("glazec: Successfully generated {}\n", header_path.string());
            return 0;
        }

        const std::filesystem::path& deps_path = *deps_path_opt;
        const std::filesystem::path glm_path = deps_path / "glm-src";
        const std::filesystem::path entt_path = deps_path / "entt-src/src";

        const std::string command = std::format(
            R"(g++ -std=c++20 -fsyntax-only -x c++-header -I"{}" -I"{}" -I"{}" -I"{}" "{}")",
            root_path.string(),
            include_path.string(),
            glm_path.string(),
            entt_path.string(),
            std::filesystem::absolute(header_path).lexically_normal().string()
        );

        int compile_exit_code = std::system(command.c_str());
        if (compile_exit_code != 0) {
            std::cerr << std::format("glazec: C++ syntax check failed for {}\n", header_path.filename().string());
            return compile_exit_code;
        }

        std::cout << std::format("glazec: Syntax check passed. Successfully generated {}\n", header_path.string());
        return 0;
    }

    std::optional<std::filesystem::path> Compiler::resolve_fetch_content_deps(const std::filesystem::path& root_path) const {
        if (const auto explicit_build = get_env_var("GLACIAL_BUILD_DIR")) {
            std::filesystem::path custom_deps = root_path / *explicit_build / "_deps";
            if (std::filesystem::exists(custom_deps)) {
                return custom_deps;
            }
        }

        const std::vector<std::string> common_build_dirs = {
            "cmake-build-debug/_deps",
            "cmake-build-release/_deps",
            "build/release/_deps",
            "build/debug/_deps"
        };

        for (const auto& relative : common_build_dirs) {
            std::filesystem::path candidate = root_path / relative;
            if (std::filesystem::exists(candidate)) {
                return candidate;
            }
        }

        return std::nullopt;
    }
}
