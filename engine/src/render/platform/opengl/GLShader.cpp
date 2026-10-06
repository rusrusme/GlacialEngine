//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/platform/opengl/GLShader.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_set>
#include <vector>
#include <glad/glad.h>

#include "core/Log.hpp"

namespace engine {
    static GLenum shader_type_from_string(const std::string& type) {
        if (type == "vertex") return GL_VERTEX_SHADER;
        if (type == "fragment" || type == "pixel") return GL_FRAGMENT_SHADER;
        if (type == "geometry") return GL_GEOMETRY_SHADER;
        if (type == "compute") return GL_COMPUTE_SHADER;

        CORE_ERROR("[GLShader] Unknown shader stage type: {0}", type);
        return 0;
    }

    GLShader::GLShader(const std::string& filepath) {
        std::unordered_set<std::string> included_files;
        const std::string source = preprocess_source(filepath, included_files);

        std::filesystem::path path(filepath);
        name_ = path.stem().string();

        const auto shader_sources = parse_shader_stages(source);
        compile_and_link(shader_sources);
    }

    GLShader::GLShader(std::string name, const std::string& vertex_src, const std::string& fragment_src)
        : name_(std::move(name)) {
        std::unordered_map<uint32_t, std::string> sources;
        sources[GL_VERTEX_SHADER] = vertex_src;
        sources[GL_FRAGMENT_SHADER] = fragment_src;

        compile_and_link(sources);
    }

    GLShader::GLShader(std::string name, const std::string& compute_src)
        : name_(std::move(name)) {
        std::unordered_map<uint32_t, std::string> sources;
        sources[GL_COMPUTE_SHADER] = compute_src;

        compile_and_link(sources);
    }

    GLShader::~GLShader() {
        if (renderer_id_ != 0) {
            glDeleteProgram(renderer_id_);
        }
    }

    GLShader::GLShader(GLShader&& other) noexcept
        : renderer_id_(other.renderer_id_), name_(std::move(other.name_)) {
        other.renderer_id_ = 0;
    }

    GLShader& GLShader::operator=(GLShader&& other) noexcept {
        if (this != &other) {
            if (renderer_id_ != 0) {
                glDeleteProgram(renderer_id_);
            }
            renderer_id_ = other.renderer_id_;
            name_ = std::move(other.name_);
            other.renderer_id_ = 0;
        }
        return *this;
    }

    void GLShader::bind() const {
        glUseProgram(renderer_id_);
    }

    void GLShader::unbind() const {
        glUseProgram(0);
    }

    std::string GLShader::read_file(const std::string& filepath) {
        std::ifstream in(filepath, std::ios::in | std::ios::binary);
        if (!in) {
            std::cerr << "[GLShader] Could not open file: " << filepath << "\n";
            return "";
        }

        std::string result;
        in.seekg(0, std::ios::end);
        const auto size = in.tellg();
        if (size != -1) {
            result.resize(size);
            in.seekg(0, std::ios::beg);
            in.read(&result[0], size);
        }
        else {
            std::cerr << "[GLShader] Could not read file: " << filepath << "\n";
        }
        return result;
    }

    std::unordered_map<uint32_t, std::string> GLShader::parse_shader_stages(const std::string& source) {
        std::unordered_map<uint32_t, std::string> shader_sources;

        auto type_token = "#type";
        const size_t type_token_length = strlen(type_token);
        size_t pos = source.find(type_token, 0);

        while (pos != std::string::npos) {
            const size_t eol = source.find_first_of("\r\n", pos);
            const size_t begin = pos + type_token_length + 1;
            std::string type = source.substr(begin, eol - begin);

            const size_t next_line_pos = source.find_first_not_of("\r\n", eol);
            pos = source.find(type_token, next_line_pos);

            shader_sources[shader_type_from_string(type)] = pos == std::string::npos
                                                                ? source.substr(next_line_pos)
                                                                : source.substr(next_line_pos, pos - next_line_pos);
        }

        return shader_sources;
    }

    std::string GLShader::preprocess_source(const std::string& filepath, std::unordered_set<std::string>& included_files) {
        std::string canonical_path = std::filesystem::weakly_canonical(filepath).string();

        if (included_files.contains(canonical_path)) {
            return "";
        }

        if (!std::filesystem::exists(filepath)) {
            std::cerr << "[GLShader] Include File does not exist: " << filepath << "\n";
            return "";
        }

        included_files.insert(canonical_path);

        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "[GLShader] Could not open include file: " << filepath << "\n";
            return "";
        }

        std::string directory;
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
                    std::string full_include_path = directory + include_rel_path;

                    if (!std::filesystem::exists(full_include_path)) {
                        full_include_path = "shaders/" + include_rel_path; // Root Fallback
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

    uint32_t GLShader::compile_spirv_shader(unsigned int type, const std::vector<uint8_t>& spirv_data) {
        const uint32_t shader = glCreateShader(type);

        glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv_data.data(), static_cast<GLsizei>(spirv_data.size()));
        glSpecializeShader(shader, "main", 0, nullptr, nullptr);

        int success = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char info_log[512];
            glGetShaderInfoLog(shader, 512, nullptr, info_log);
            CORE_ERROR("SPIR-V Shader specialization failed:\n{0}", info_log);
            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

    void GLShader::compile_and_link(const std::unordered_map<uint32_t, std::string>& sources) {
        const GLuint program = glCreateProgram();
        std::vector<GLuint> gl_shader_ids;
        gl_shader_ids.reserve(sources.size());

        for (auto&& [stage_type, stage_source] : sources) {
            const GLuint shader = glCreateShader(stage_type);
            const GLchar* source_cstr = stage_source.c_str();
            glShaderSource(shader, 1, &source_cstr, nullptr);
            glCompileShader(shader);

            GLint is_compiled = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled);
            if (is_compiled == GL_FALSE) {
                GLint max_length = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length);

                std::vector<GLchar> info_log(max_length);
                glGetShaderInfoLog(shader, max_length, &max_length, info_log.data());

                glDeleteShader(shader);
                std::cerr << "[GLShader] Compilation error in " << name_ << ":\n" << info_log.data() << "\n";
                break;
            }

            glAttachShader(program, shader);
            gl_shader_ids.push_back(shader);
        }

        glLinkProgram(program);

        GLint is_linked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &is_linked);
        if (is_linked == GL_FALSE) {
            GLint max_length = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_length);

            std::vector<GLchar> info_log(max_length);
            glGetProgramInfoLog(program, max_length, &max_length, info_log.data());

            glDeleteProgram(program);
            for (const auto id : gl_shader_ids) {
                glDeleteShader(id);
            }

            std::cerr << "[GLShader] Linking error in " << name_ << ":\n" << info_log.data() << "\n";
            return;
        }

        for (const auto id : gl_shader_ids) {
            glDetachShader(program, id);
            glDeleteShader(id);
        }

        renderer_id_ = program;
    }
} // engine
