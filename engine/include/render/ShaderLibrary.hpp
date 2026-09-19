//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "render/Shader.hpp"

namespace engine {
    class ShaderLibrary {
    public:
        ShaderLibrary() = default;
        ~ShaderLibrary() = default;

        ShaderLibrary(const ShaderLibrary&) = delete;
        ShaderLibrary& operator=(const ShaderLibrary&) = delete;

        void add(const std::string& name, const std::shared_ptr<Shader>& shader);

        std::shared_ptr<Shader> load(const std::string& name, const std::string& vertex_path, const std::string& fragment_path, const std::string& geometry_path = "");

        [[nodiscard]] std::shared_ptr<Shader> get(const std::string& name) const;

        [[nodiscard]] bool exists(const std::string& name) const;

        [[nodiscard]] const std::unordered_map<std::string, std::shared_ptr<Shader>>& get_shaders() const { return shaders_; }

    private:
        std::unordered_map<std::string, std::shared_ptr<Shader>> shaders_;
    };
}
