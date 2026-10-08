//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/platform/opengl/GLShader.hpp"

#include "core/Log.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <glad/glad.h>

namespace engine {
    static GLenum shader_type_from_string(const std::string& type) {
        if (type == "vertex") return GL_VERTEX_SHADER;
        if (type == "fragment" || type == "pixel") return GL_FRAGMENT_SHADER;
        if (type == "geometry") return GL_GEOMETRY_SHADER;
        if (type == "compute") return GL_COMPUTE_SHADER;

        CORE_ERROR("[GLShader] Unknown shader stage type: '{0}'", type);
        return 0;
    }

    GLShader::GLShader(const std::string& filepath) {
        std::unordered_set<std::string> included_files;
        const std::string source = preprocess_source(filepath, included_files);

        std::filesystem::path path(filepath);
        name_ = path.stem().string();

        const auto shader_sources = parse_shader_stages(source);
        compile(shader_sources);
    }

    GLShader::GLShader(std::string name, const LoadedBSHData& bsh_data)
        : name_(std::move(name)) {
        compile_bsh(bsh_data);
    }

    GLShader::GLShader(std::string name, const std::string& vertex_src, const std::string& fragment_src)
        : name_(std::move(name)) {
        std::unordered_map<uint32_t, std::string> sources;
        sources[GL_VERTEX_SHADER] = vertex_src;
        sources[GL_FRAGMENT_SHADER] = fragment_src;

        compile(sources);
    }

    GLShader::GLShader(std::string name, const std::string& compute_src)
        : name_(std::move(name)) {
        std::unordered_map<uint32_t, std::string> sources;
        sources[GL_COMPUTE_SHADER] = compute_src;

        compile(sources);
    }

    GLShader::GLShader(std::string name, const std::unordered_map<uint32_t, std::vector<uint8_t>>& spirv_sources)
        : name_(std::move(name)) {
        compile_spirv(spirv_sources);
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
            CORE_ERROR("[GLShader] Could not open file: '{0}'", filepath);
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
            CORE_ERROR("[GLShader] Could not read file content: '{0}'", filepath);
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

            const uint32_t stage = shader_type_from_string(type);
            if (stage != 0) {
                shader_sources[stage] = pos == std::string::npos
                                            ? source.substr(next_line_pos)
                                            : source.substr(next_line_pos, pos - next_line_pos);
            }
        }

        return shader_sources;
    }

    std::string GLShader::preprocess_source(const std::string& filepath, std::unordered_set<std::string>& included_files) {
        std::string canonical_path = std::filesystem::weakly_canonical(filepath).string();

        if (included_files.contains(canonical_path)) {
            return ""; // Avoid duplicate inclusion
        }

        if (!std::filesystem::exists(filepath)) {
            CORE_ERROR("[GLShader] Include file does not exist: '{0}'", filepath);
            return "";
        }

        included_files.insert(canonical_path);

        std::ifstream file(filepath);
        if (!file.is_open()) {
            CORE_ERROR("[GLShader] Could not open include file: '{0}'", filepath);
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

    uint32_t GLShader::compile_spirv_shader(uint32_t type, const std::vector<uint8_t>& spirv_data) {
        const uint32_t shader = glCreateShader(type);

        glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv_data.data(), static_cast<GLsizei>(spirv_data.size()));
        glSpecializeShader(shader, "main", 0, nullptr, nullptr);

        GLint success = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (success == GL_FALSE) {
            GLint max_length = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length);

            std::vector<GLchar> info_log(max_length);
            glGetShaderInfoLog(shader, max_length, &max_length, info_log.data());

            CORE_ERROR("[GLShader] SPIR-V specialization failed for '{0}':\n{1}", name_, info_log.data());
            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

    void GLShader::compile_bsh(const LoadedBSHData& bsh_data) {
        if (!bsh_data.valid) {
            return;
        }

        auto to_gl_stage = [](bshc::ShaderStage stage) -> uint32_t {
            switch (stage) {
                case bshc::ShaderStage::Vertex : return GL_VERTEX_SHADER;
                case bshc::ShaderStage::Fragment : return GL_FRAGMENT_SHADER;
                case bshc::ShaderStage::Geometry : return GL_GEOMETRY_SHADER;
                case bshc::ShaderStage::Compute : return GL_COMPUTE_SHADER;
            }
            return 0;
        };

        if (bsh_data.backend == bshc::RenderBackend::OpenGL) {
            std::unordered_map<uint32_t, std::string> sources;
            for (const auto& stage : bsh_data.stages) {
                if (uint32_t gl_stage = to_gl_stage(stage.stage)) {
                    sources[gl_stage] = std::string(stage.as_string_view());
                }
            }
            compile(sources);
        }
        else if (bsh_data.backend == bshc::RenderBackend::Vulkan) {
            std::unordered_map<uint32_t, std::vector<uint8_t>> spirv_sources;
            for (const auto& stage : bsh_data.stages) {
                if (uint32_t gl_stage = to_gl_stage(stage.stage)) {
                    spirv_sources[gl_stage] = stage.code;
                }
            }
            compile_spirv(spirv_sources);
        }
    }

    void GLShader::compile(const std::unordered_map<uint32_t, std::string>& sources) {
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
                CORE_ERROR("[GLShader] Compilation error in '{0}':\n{1}", name_, info_log.data());
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

            CORE_ERROR("[GLShader] Linking error in '{0}':\n{1}", name_, info_log.data());
            return;
        }

        for (const auto id : gl_shader_ids) {
            glDetachShader(program, id);
            glDeleteShader(id);
        }

        renderer_id_ = program;
    }

    void GLShader::compile_spirv(const std::unordered_map<uint32_t, std::vector<uint8_t>>& spirv_sources) {
        const GLuint program = glCreateProgram();
        std::vector<GLuint> gl_shader_ids;
        gl_shader_ids.reserve(spirv_sources.size());

        for (auto&& [stage_type, spirv_data] : spirv_sources) {
            const uint32_t shader = compile_spirv_shader(stage_type, spirv_data);
            if (shader != 0) {
                glAttachShader(program, shader);
                gl_shader_ids.push_back(shader);
            }
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

            CORE_ERROR("[GLShader] SPIR-V Linking error in '{0}':\n{1}", name_, info_log.data());
            return;
        }

        for (const auto id : gl_shader_ids) {
            glDetachShader(program, id);
            glDeleteShader(id);
        }

        renderer_id_ = program;
    }
} // namespace engine
