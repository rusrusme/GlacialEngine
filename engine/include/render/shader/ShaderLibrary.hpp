//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>

#include "render/rhi/RHITypes.hpp"
#include "render/rhi/Shader.hpp"

namespace engine {
    struct StringHash {
        using is_transparent = void;
        size_t operator()(std::string_view sv) const {
            return std::hash<std::string_view>{}(sv);
        }
    };
    using ShaderMap = std::unordered_map<std::string, Shared<Shader>, StringHash, std::equal_to<>>;

    class ShaderLibrary {
    public:
        ShaderLibrary() = default;
        ~ShaderLibrary() = default;

        ShaderLibrary(const ShaderLibrary&) = delete;
        ShaderLibrary& operator=(const ShaderLibrary&) = delete;

        void add(const std::string& name, const Shared<Shader>& shader);
        void remove(std::string_view name);

        Shared<Shader> load(std::string_view name, const std::string& shader_path);

        bool reload(std::string_view name);
        void reload_all();

        void clear();
        void clear_unused();

        [[nodiscard]] Shared<Shader> get(std::string_view name) const;
        [[nodiscard]] bool exists(std::string_view name) const;

        [[nodiscard]] const ShaderMap& shaders() const { return shaders_; }

    private:
        mutable std::shared_mutex mutex_;

        ShaderMap shaders_;
    };
} // engine
