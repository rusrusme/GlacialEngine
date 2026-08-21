//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/ShaderLibrary.hpp"

namespace engine {
    void ShaderLibrary::add(const std::string& name, const std::shared_ptr<Shader>& shader) {
        if (exists(name)) {
            std::cerr << "Engine warning: shader '" << name << "' is already registered" << std::endl;
            return;
        }
        shaders_[name] = shader;
    }

    std::shared_ptr<Shader> ShaderLibrary::load(const std::string& name, const std::string& vertex_path, const std::string& fragment_path) {
        if (exists(name)) {
            return shaders_[name];
        }

        auto shader = Shader::from_file(vertex_path, fragment_path);
        if (!shader) {
            std::cerr << "Engine error: failed to load shader '" << name << "' from files" << std::endl;
            return nullptr;
        }

        // convert unique_ptr to shared_ptr for caching
        std::shared_ptr shared_shader = std::move(shader);
        shaders_[name] = shared_shader;
        return shared_shader;
    }

    std::shared_ptr<Shader> ShaderLibrary::get(const std::string& name) const {
        auto it = shaders_.find(name);
        if (it == shaders_.end()) {
            std::cerr << "Engine error: shader '" << name << "' not found in library" << std::endl;
            return nullptr;
        }
        return it->second;
    }

    bool ShaderLibrary::exists(const std::string& name) const {
        return shaders_.contains(name);
    }
}
