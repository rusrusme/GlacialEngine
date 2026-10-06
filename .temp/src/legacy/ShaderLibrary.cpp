//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/legacy/ShaderLibrary.hpp"

#include "core/Log.hpp"

namespace engine::legacy {
    using namespace engine;
    void ShaderLibrary::add(const std::string& name, const std::shared_ptr<Shader>& shader) {
        if (exists(name)) {
            CORE_WARN("Shader '{0}' is already registered", name);
            return;
        }
        shaders_[name] = shader;
    }

    std::shared_ptr<Shader> ShaderLibrary::load(const std::string& name, const std::string& vertex_path, const std::string& fragment_path, const std::string& geometry_path) {
        if (exists(name)) {
            return shaders_[name];
        }

        auto shader = Shader::from_file(vertex_path, fragment_path, geometry_path);
        if (!shader) {
            CORE_ERROR("Failed to load shader '{0}' from files", name);
            return nullptr;
        }

        // convert unique_ptr to shared_ptr for caching
        std::shared_ptr shared_shader = std::move(shader);
        shaders_[name] = shared_shader;
        return shared_shader;
    }

    std::shared_ptr<Shader> ShaderLibrary::load(const std::string& name, const std::string& bsh_path) {
        if (exists(name)) {
            return shaders_[name];
        }

        auto shader = Shader::from_bsh(bsh_path);
        if (!shader) {
            CORE_ERROR("Failed to load BSH shader '{0}' from file '{1}'", name, bsh_path);
            return nullptr;
        }

        std::shared_ptr shared_shader = std::move(shader);
        shaders_[name] = shared_shader;
        return shared_shader;
    }

    std::shared_ptr<Shader> ShaderLibrary::get(const std::string& name) const {
        auto it = shaders_.find(name);
        if (it == shaders_.end()) {
            CORE_ERROR("Shader '{0}' not found in library", name);
            return nullptr;
        }
        return it->second;
    }

    bool ShaderLibrary::exists(const std::string& name) const {
        return shaders_.contains(name);
    }
}
