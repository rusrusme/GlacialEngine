//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/Shader.hpp"

#include "util/FileIO.hpp"

namespace engine {
    Shader::Shader(const std::string& vertex_src, const std::string& fragment_src) {
        const uint32_t vs = compile_shader(GL_VERTEX_SHADER, vertex_src);
        const uint32_t fs = compile_shader(GL_FRAGMENT_SHADER, fragment_src);

        if (vs == 0 || fs == 0) return;

        id_ = glCreateProgram();
        glAttachShader(id_, vs);
        glAttachShader(id_, fs);
        glLinkProgram(id_);

        int success = 0;
        glGetProgramiv(id_, GL_LINK_STATUS, &success);
        if (!success) {
            char info_log[512];
            glGetProgramInfoLog(id_, 512, nullptr, info_log);
            std::cerr << "Engine error: shader program linking failed\n" << info_log << std::endl;
            glDeleteProgram(id_);
            id_ = 0;
        }

        glDetachShader(id_, vs);
        glDetachShader(id_, fs);
        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    Shader::~Shader() {
        if (id_ != 0) {
            glDeleteProgram(id_);
        }
    }

    Shader::Shader(Shader&& other) noexcept
        : id_(std::exchange(other.id_, 0)),
          uniform_location_cache_(std::move(other.uniform_location_cache_)) {}

    Shader& Shader::operator=(Shader&& other) noexcept {
        if (this != &other) {
            if (id_ != 0)
                glDeleteProgram(id_);
            id_ = std::exchange(other.id_, 0);
            uniform_location_cache_ = std::move(other.uniform_location_cache_);
        }
        return *this;
    }

    std::unique_ptr<Shader> Shader::from_file(const std::string& vertex_path, const std::string& fragment_path) {
        std::unordered_set<std::string> vs_includes;
        std::unordered_set<std::string> fs_includes;
        const std::string vs_code = preprocess_source(vertex_path, vs_includes);
        const std::string fs_code = preprocess_source(fragment_path, fs_includes);

        if (vs_code.empty() || fs_code.empty()) {
            std::cerr << "Engine error: failed to load shader sources from files!" << std::endl;
            return nullptr;
        }

        return std::make_unique<Shader>(vs_code, fs_code);
    }

    void Shader::bind() const {
        glUseProgram(id_);
    }

    void Shader::unbind() const {
        glUseProgram(0);
    }

    void Shader::set_int(const std::string& name, const int value) {
        glUniform1i(get_uniform_location(name), value);
    }

    void Shader::set_float(const std::string& name, const float value) {
        glUniform1f(get_uniform_location(name), value);
    }

    void Shader::set_vec2(const std::string& name, const glm::vec2& value) {
        glUniform2fv(get_uniform_location(name), 1, glm::value_ptr(value));
    }

    void Shader::set_vec3(const std::string& name, const glm::vec3& value) {
        glUniform3fv(get_uniform_location(name), 1, glm::value_ptr(value));
    }

    void Shader::set_vec4(const std::string& name, const glm::vec4& value) {
        glUniform4fv(get_uniform_location(name), 1, glm::value_ptr(value));
    }

    void Shader::set_mat4(const std::string& name, const glm::mat4& value) {
        glUniformMatrix4fv(get_uniform_location(name), 1, GL_FALSE, glm::value_ptr(value));
    }

    int Shader::get_uniform_location(const std::string& name) const {
        if (const auto it = uniform_location_cache_.find(name); it != uniform_location_cache_.end()) {
            return it->second;
        }

        const int location = glGetUniformLocation(id_, name.c_str());
        if (location == -1) {
            std::cerr << "Engine warning: uniform '" << name << "' does not exist or was stripped!" << std::endl;
        }
        uniform_location_cache_[name] = location;
        return location;
    }

    uint32_t Shader::compile_shader(const unsigned int type, const std::string& source) {
        const uint32_t shader = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        int success = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char info_log[512];
            glGetShaderInfoLog(shader, 512, nullptr, info_log);
            std::cerr << "Engine error: " << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment")
                << " shader compilation failed\n" << info_log << std::endl;
            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

    std::string Shader::preprocess_source(const std::string& filepath, std::unordered_set<std::string>& included_files) {
        if (included_files.contains(filepath)) {
            return ""; // Avoid duplicate inclusion
        }

        if (!std::filesystem::exists(filepath)) {
            std::cerr << "Engine error: failed to open shader file: " << filepath << std::endl;
            return "";
        }

        included_files.insert(filepath);

        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Engine error: failed to open shader file: " << filepath << std::endl;
            return "";
        }

        std::string directory = "";
        const size_t last_slash = filepath.find_last_of("/\\");
        if (last_slash != std::string::npos) {
            directory = filepath.substr(0, last_slash + 1);
        }

        std::stringstream ss;
        std::string line;

        while (std::getline(file, line)) {
            size_t include_pos = line.find("#include");
            if (include_pos != std::string::npos) {
                size_t first_quote = line.find_first_of("\"<", include_pos);
                size_t last_quote = line.find_last_of("\">");

                if (first_quote != std::string::npos && last_quote != std::string::npos && last_quote > first_quote) {
                    std::string include_rel_path = line.substr(first_quote + 1, last_quote - first_quote - 1);

                    // Try relative path resolving
                    std::string full_include_path = directory + include_rel_path;

                    // Fallback searching in root shader directory
                    if (!std::filesystem::exists(full_include_path)) {
                        full_include_path = "assets/shaders/" + include_rel_path;
                    }

                    ss << "// --- Begin Include: " << include_rel_path << " ---\n";
                    ss << preprocess_source(full_include_path, included_files) << "\n";
                    ss << "// --- End Include: " << include_rel_path << " ---\n";
                    continue;
                }
            }
            ss << line << "\n";
        }

        return ss.str();
    }
}
