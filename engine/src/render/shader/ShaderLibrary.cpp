//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/shader/ShaderLibrary.hpp"

#include "core/Log.hpp"

#include <mutex>
#include <ranges>

namespace engine {
    void ShaderLibrary::add(const std::string& name, const Shared<Shader>& shader) {
        std::unique_lock lock(mutex_);

        if (shaders_.contains(name)) {
            CORE_WARN("Shader '{0}' is already registered in library", name);
            return;
        }
        shaders_[name] = shader;
    }

    void ShaderLibrary::remove(std::string_view name) {
        std::unique_lock lock(mutex_);

        if (auto it = shaders_.find(name); it != shaders_.end()) {
            shaders_.erase(it);
        }
        else {
            CORE_WARN("Shader '{0}' not found in library for removal", name);
        }
    }

    Shared<Shader> ShaderLibrary::load(std::string_view name, const std::string& shader_path) {
        std::unique_lock lock(mutex_);

        if (auto it = shaders_.find(name); it != shaders_.end()) {
            return it->second;
        }

        auto shader = std::make_shared<Shader>(shader_path);
        if (shader->renderer_id() == 0) {
            CORE_ERROR("Failed to load BSH shader '{0}' from path '{1}'", name, shader_path);
            return nullptr;
        }
        std::string key(name);
        shaders_[key] = shader;
        return shader;
    }

    bool ShaderLibrary::reload(std::string_view name) {
        Shared<Shader> shader = nullptr;
        {
            std::shared_lock lock(mutex_);
            auto it = shaders_.find(name);
            if (it == shaders_.end()) {
                CORE_ERROR("Cannot reload shader '{0}': not found in library", name);
                return false;
            }
            shader = it->second;
        }
        return shader->reload();
    }

    void ShaderLibrary::reload_all() {
        std::vector<Shared<Shader>> target_shaders;
        {
            std::shared_lock lock(mutex_);
            target_shaders.reserve(shaders_.size());
            for (const auto& shader : shaders_ | std::views::values) {
                target_shaders.push_back(shader);
            }
        }
        for (const auto& shader : target_shaders) {
            if (shader) {
                shader->reload();
            }
        }
    }

    void ShaderLibrary::clear() {
        std::unique_lock lock(mutex_);
        shaders_.clear();
    }

    void ShaderLibrary::clear_unused() {
        std::unique_lock lock(mutex_);
        std::erase_if(shaders_, [](const auto& pair) {
            return pair.second.use_count() == 1;
        });
    }

    Shared<Shader> ShaderLibrary::get(std::string_view name) const {
        std::shared_lock lock(mutex_);

        auto it = shaders_.find(name);
        if (it == shaders_.end()) {
            CORE_ERROR("Shader '{0}' not found in library", name);
            return nullptr;
        }
        return it->second;
    }

    bool ShaderLibrary::exists(std::string_view name) const {
        std::shared_lock lock(mutex_);
        return shaders_.contains(name);
    }
}
