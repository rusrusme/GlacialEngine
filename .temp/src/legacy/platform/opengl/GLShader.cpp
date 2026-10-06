//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/legacy/platform/opengl/GLShader.hpp"

#include "core/Log.hpp"
#include "render/legacy/BSHLoader.hpp"
#include "util/FileIO.hpp"

namespace engine::legacy {
    using namespace engine;
    GLShader::GLShader(const std::string& vertex_src, const std::string& fragment_src, const std::string& geometry_src) {
        const uint32_t vs = compile_shader(GL_VERTEX_SHADER, vertex_src);
        const uint32_t fs = compile_shader(GL_FRAGMENT_SHADER, fragment_src);

        uint32_t gs = 0;
        if (!geometry_src.empty()) {
            gs = compile_shader(GL_GEOMETRY_SHADER, geometry_src);
        }

        if (vs == 0 || fs == 0 || (!geometry_src.empty() && gs == 0)) {
            if (vs != 0)
                glDeleteShader(vs);
            if (fs != 0)
                glDeleteShader(fs);
            if (gs != 0)
                glDeleteShader(gs);
            return;
        }

        id_ = glCreateProgram();
        glAttachShader(id_, vs);
        if (gs != 0)
            glAttachShader(id_, gs);
        glAttachShader(id_, fs);
        glLinkProgram(id_);

        int success = 0;
        glGetProgramiv(id_, GL_LINK_STATUS, &success);
        if (!success) {
            char info_log[512];
            glGetProgramInfoLog(id_, 512, nullptr, info_log);
            CORE_ERROR("Shader program linking failed:\n{0}", info_log);
            glDeleteProgram(id_);
            id_ = 0;
        }

        glDetachShader(id_, vs);
        if (gs != 0)
            glDetachShader(id_, gs);
        glDetachShader(id_, fs);
        glDeleteShader(vs);
        if (gs != 0)
            glDeleteShader(gs);
        glDeleteShader(fs);
    }

    GLShader::GLShader(const LoadedBSHData& bsh_data)
        : header_(bsh_data.header) {
        if (!bsh_data.valid) {
            CORE_ERROR("Attempted to construct GLShader with invalid LoadedBSHData");
            return;
        }

        std::vector<uint32_t> attached_shaders;
        id_ = glCreateProgram();

        for (const auto& stage : bsh_data.stages) {
            GLenum gl_type = GL_VERTEX_SHADER;
            switch (stage.stage_type) {
                case bshc::ShaderStage::VERTEX : gl_type = GL_VERTEX_SHADER;
                    break;
                case bshc::ShaderStage::FRAGMENT : gl_type = GL_FRAGMENT_SHADER;
                    break;
                case bshc::ShaderStage::GEOMETRY : gl_type = GL_GEOMETRY_SHADER;
                    break;
                case bshc::ShaderStage::COMPUTE : gl_type = GL_COMPUTE_SHADER;
                    break;
            }

            uint32_t shader_id = compile_spirv_shader(gl_type, stage.data);
            if (shader_id != 0) {
                glAttachShader(id_, shader_id);
                attached_shaders.push_back(shader_id);
            }
        }

        glLinkProgram(id_);

        int success = 0;
        glGetProgramiv(id_, GL_LINK_STATUS, &success);
        if (!success) {
            char info_log[512];
            glGetProgramInfoLog(id_, 512, nullptr, info_log);
            CORE_ERROR("SPIR-V Shader program linking failed:\n{0}", info_log);
            glDeleteProgram(id_);
            id_ = 0;
        }

        for (uint32_t shader_id : attached_shaders) {
            glDetachShader(id_, shader_id);
            glDeleteShader(shader_id);
        }
    }

    GLShader::~GLShader() {
        if (id_ != 0) {
            glDeleteProgram(id_);
        }
    }

    GLShader::GLShader(GLShader&& other) noexcept
        : id_(std::exchange(other.id_, 0)),
          header_(other.header_),
          uniform_location_cache_(std::move(other.uniform_location_cache_)) {}

    GLShader& GLShader::operator=(GLShader&& other) noexcept {
        if (this != &other) {
            if (id_ != 0)
                glDeleteProgram(id_);
            id_ = std::exchange(other.id_, 0);
            header_ = other.header_;
            uniform_location_cache_ = std::move(other.uniform_location_cache_);
        }
        return *this;
    }

    std::unique_ptr<GLShader> GLShader::from_file(const std::string& vertex_path, const std::string& fragment_path, const std::string& geometry_path) {
        std::unordered_set<std::string> vs_includes;
        std::unordered_set<std::string> fs_includes;
        std::unordered_set<std::string> gs_includes;

        const std::string vs_code = preprocess_source(vertex_path, vs_includes);
        if (vs_code.empty()) {
            CORE_ERROR("Failed to load vertex shader source: '{0}'", vertex_path);
            return nullptr;
        }

        const std::string fs_code = preprocess_source(fragment_path, fs_includes);
        if (fs_code.empty()) {
            CORE_ERROR("Failed to load fragment shader source: '{0}'", fragment_path);
            return nullptr;
        }

        std::string gs_code;
        if (!geometry_path.empty()) {
            gs_code = preprocess_source(geometry_path, gs_includes);
            if (gs_code.empty()) {
                CORE_ERROR("Failed to load geometry shader source: '{0}'", geometry_path);
                return nullptr;
            }
        }

        return std::make_unique<GLShader>(vs_code, fs_code, gs_code);
    }

    std::unique_ptr<GLShader> GLShader::from_bsh(const std::string& bsh_path) {
        LoadedBSHData data = BSHLoader::load_from_file(bsh_path);
        if (!data.valid) {
            CORE_ERROR("Failed to load BSH shader asset: {0}", bsh_path);
            return nullptr;
        }
        return std::make_unique<GLShader>(data);
    }

    void GLShader::bind() const {
        glUseProgram(id_);
    }

    void GLShader::unbind() const {
        glUseProgram(0);
    }

    void GLShader::set_int(const std::string& name, const int value) const {
        glUniform1i(get_uniform_location(name), value);
    }

    void GLShader::set_float(const std::string& name, const float value) const {
        glUniform1f(get_uniform_location(name), value);
    }

    void GLShader::set_vec2(const std::string& name, const glm::vec2& value) const {
        glUniform2fv(get_uniform_location(name), 1, glm::value_ptr(value));
    }

    void GLShader::set_vec3(const std::string& name, const glm::vec3& value) const {
        glUniform3fv(get_uniform_location(name), 1, glm::value_ptr(value));
    }

    void GLShader::set_vec4(const std::string& name, const glm::vec4& value) const {
        glUniform4fv(get_uniform_location(name), 1, glm::value_ptr(value));
    }

    void GLShader::set_mat4(const std::string& name, const glm::mat4& value) const {
        glUniformMatrix4fv(get_uniform_location(name), 1, GL_FALSE, glm::value_ptr(value));
    }

    int GLShader::get_uniform_location(const std::string& name) const {
        if (const auto it = uniform_location_cache_.find(name); it != uniform_location_cache_.end()) {
            return it->second;
        }

        const int location = glGetUniformLocation(id_, name.c_str());
        if (location == -1) {
            CORE_WARN("Uniform '{0}' does not exist or was stripped", name);
        }
        uniform_location_cache_[name] = location;
        return location;
    }

    uint32_t GLShader::compile_shader(const unsigned int type, const std::string& source) {
        const uint32_t shader = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        int success = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char info_log[512];
            glGetShaderInfoLog(shader, 512, nullptr, info_log);

            const std::string type_name =
                type == GL_VERTEX_SHADER ? "Vertex" : type == GL_GEOMETRY_SHADER ? "Geometry" : "Fragment";

            CORE_ERROR("{0} shader compilation failed:\n{1}", type_name, info_log);
            glDeleteShader(shader);
            return 0;
        }

        return shader;
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

    std::string GLShader::preprocess_source(const std::string& filepath, std::unordered_set<std::string>& included_files) {
        std::string canonical_path = std::filesystem::weakly_canonical(filepath).string();

        if (included_files.contains(canonical_path)) {
            return ""; // Avoid duplicate inclusion
        }

        if (!std::filesystem::exists(filepath)) {
            CORE_ERROR("Failed to open shader file: {0}", filepath);
            return "";
        }

        included_files.insert(canonical_path);

        std::ifstream file(filepath);
        if (!file.is_open()) {
            CORE_ERROR("Failed to open shader file: {0}", filepath);
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
