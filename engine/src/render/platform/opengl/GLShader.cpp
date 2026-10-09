//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/platform/opengl/GLShader.hpp"

#include <filesystem>
#include <utility>
#include <vector>
#include <glad/glad.h>

#include "core/Log.hpp"

namespace engine {
    GLShader::GLShader(std::string filepath)
        : filepath_(std::move(filepath)) {
        const std::filesystem::path path(filepath_);
        name_ = path.stem().string();

        const LoadedBSHData bsh_data = BSHLoader::load_from_binary(filepath_);
        if (!bsh_data.valid) {
            CORE_ERROR("[GLShader] Failed to load BSHB file '{0}': {1}", filepath_, bsh_data.error_message);
            return;
        }
        load(bsh_data);
    }

    GLShader::~GLShader() {
        if (renderer_id_ != 0) {
            glDeleteProgram(renderer_id_);
        }
    }

    GLShader::GLShader(GLShader&& other) noexcept
        : renderer_id_(other.renderer_id_),
          name_(std::move(other.name_)),
          filepath_(std::move(other.filepath_)) {
        other.renderer_id_ = 0;
    }

    GLShader& GLShader::operator=(GLShader&& other) noexcept {
        if (this != &other) {
            if (renderer_id_ != 0) {
                glDeleteProgram(renderer_id_);
            }
            renderer_id_ = other.renderer_id_;
            name_ = std::move(other.name_);
            filepath_ = std::move(other.filepath_);
            other.renderer_id_ = 0;
        }
        return *this;
    }

    bool GLShader::reload() {
        if (filepath_.empty()) {
            CORE_WARN("[GLShader] Cannot reload shader '{0}': No source filepath stored", name_);
            return false;
        }

        const LoadedBSHData bsh_data = BSHLoader::load_from_binary(filepath_);
        if (!bsh_data.valid) {
            CORE_ERROR("[GLShader] Reload failed for '{0}': {1}", name_, bsh_data.error_message);
            return false;
        }

        const uint32_t old_program_id = renderer_id_;
        renderer_id_ = 0;

        load(bsh_data);

        if (renderer_id_ == 0) {
            renderer_id_ = old_program_id;
            CORE_ERROR("[GLShader] Reload failed for '{0}'. Keeping active program ID: {1}", name_, renderer_id_);
            return false;
        }

        if (old_program_id != 0) {
            glDeleteProgram(old_program_id);
        }
        CORE_INFO("[GLShader] Successfully reloaded shader '{0}' from '{1}'", name_, filepath_);
        return true;
    }

    void GLShader::bind() const {
        glUseProgram(renderer_id_);
    }

    void GLShader::unbind() const {
        glUseProgram(0);
    }

    void GLShader::load(const LoadedBSHData& bsh_data) {
        if (!bsh_data.valid) { return; }
        if (bsh_data.backend != bshc::RenderBackend::OpenGL) {
            CORE_ERROR("[GLShader] Cannot load BSH shader '{0}': Target backend is not OpenGL", name_);
            return;
        }

        auto to_gl_stage = [](bshc::ShaderStage stage) -> uint32_t {
            switch (stage) {
                case bshc::ShaderStage::Vertex : return GL_VERTEX_SHADER;
                case bshc::ShaderStage::Fragment : return GL_FRAGMENT_SHADER;
                case bshc::ShaderStage::Geometry : return GL_GEOMETRY_SHADER;
                case bshc::ShaderStage::Compute : return GL_COMPUTE_SHADER;
                default : return 0;
            }
        };

        std::unordered_map<uint32_t, std::string> sources;
        for (const auto& stage : bsh_data.stages) {
            if (uint32_t gl_stage = to_gl_stage(stage.stage)) {
                sources[gl_stage] = std::string(stage.as_string_view());
            }
        }
        create_program(sources);
    }

    void GLShader::create_program(const std::unordered_map<uint32_t, std::string>& sources) {
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
                for (const auto id : gl_shader_ids) {
                    glDetachShader(program, id);
                    glDeleteShader(id);
                }
                glDeleteProgram(program);

                CORE_ERROR("[GLShader] Compilation error in '{0}':\n{1}", name_, info_log.data());
                return;
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
                glDetachShader(program, id);
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
} // namespace engine
