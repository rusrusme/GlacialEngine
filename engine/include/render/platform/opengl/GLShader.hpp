//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace engine {
    class GLShader {
    public:
        GLShader() = default;
        explicit GLShader(const std::string& filepath);
        GLShader(std::string name, const std::string& vertex_src, const std::string& fragment_src);
        GLShader(std::string name, const std::string& compute_src);
        ~GLShader();

        GLShader(const GLShader&) = delete;
        GLShader& operator=(const GLShader&) = delete;

        GLShader(GLShader&& other) noexcept;
        GLShader& operator=(GLShader&& other) noexcept;

        void bind() const;
        void unbind() const;

        [[nodiscard]] const std::string& name() const { return name_; }
        [[nodiscard]] uint32_t renderer_id() const { return renderer_id_; }

    private:
        std::string read_file(const std::string& filepath);
        std::unordered_map<uint32_t, std::string> parse_shader_stages(const std::string& source);
        std::string preprocess_source(const std::string& filepath, std::unordered_set<std::string>& included_files);
        uint32_t compile_spirv_shader(unsigned int type, const std::vector<uint8_t>& spirv_data);
        void compile_and_link(const std::unordered_map<uint32_t, std::string>& sources);

        uint32_t renderer_id_ = 0;
        std::string name_;
    };
} // engine
