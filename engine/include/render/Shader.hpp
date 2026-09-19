//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <glm/glm.hpp>

namespace engine {
    class Shader {
    public:
        Shader(const std::string& vertex_src, const std::string& fragment_src);
        ~Shader();

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;

        Shader(Shader&& other) noexcept;
        Shader& operator=(Shader&& other) noexcept;

        static std::unique_ptr<Shader> from_file(const std::string& vertex_path, const std::string& fragment_path);

        [[nodiscard]] uint32_t get_renderer_id() const { return id_; }

        void bind() const;
        void unbind() const;

        // Uniform Setters
        void set_int(const std::string& name, int value);
        void set_float(const std::string& name, float value);
        void set_vec2(const std::string& name, const glm::vec2& value);
        void set_vec3(const std::string& name, const glm::vec3& value);
        void set_vec4(const std::string& name, const glm::vec4& value);
        void set_mat4(const std::string& name, const glm::mat4& value);

    private:
        int get_uniform_location(const std::string& name) const;
        uint32_t compile_shader(unsigned int type, const std::string& source);

        static std::string preprocess_source(const std::string& filepath, std::unordered_set<std::string>& included_files);

        uint32_t id_ = 0;
        mutable std::unordered_map<std::string, int> uniform_location_cache_;
    };
}
