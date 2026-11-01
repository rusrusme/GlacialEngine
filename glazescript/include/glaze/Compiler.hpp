//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace glaze {
    struct CompilerOptions {
        std::filesystem::path engine_path{"."};
        std::filesystem::path output_path{"."};
        bool extensive_syntax_check{false};
    };

    class Compiler {
    public:
        explicit Compiler(CompilerOptions options = {});

        int compile_file(const std::filesystem::path& file_path);
        int compile_source(std::string_view source_code,
                           const std::string& script_name,
                           const std::filesystem::path& output_dir);

        static bool generate_bundle(const std::filesystem::path& output_dir);

    private:
        [[nodiscard]] std::optional<std::filesystem::path> resolve_fetch_content_deps(const std::filesystem::path& root_path) const;
        [[nodiscard]] int run_syntax_check(const std::filesystem::path& header_path) const;

    private:
        CompilerOptions options_;
    };
}
