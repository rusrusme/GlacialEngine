//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <filesystem>
#include <string>
#include <string_view>

#include "UUID.hpp"

namespace engine {
    using AssetHandle = UUID;

    struct AssetType {
        uint64_t id{0};

        constexpr AssetType() = default;
        constexpr explicit AssetType(uint64_t id) : id(id) {}
        constexpr AssetType(std::string_view name) : id(hash_name(name)) {}

        [[nodiscard]] constexpr bool valid() const { return id != 0; }
        constexpr bool operator==(const AssetType&) const = default;

    private:
        static constexpr uint64_t hash_name(std::string_view name) {
            if (name.empty() || name == "None") {
                return 0;
            }
            uint64_t hash = 14695981039346656037ULL;
            for (unsigned char c : name) {
                hash ^= c;
                hash *= 1099511628211ULL;
            }
            return hash == 0 ? 1 : hash;
        }
    };

    namespace Asset {
        inline constexpr AssetType NONE{};
        inline constexpr AssetType TEXTURE_2D{"Texture2D"};
        inline constexpr AssetType TEXTURE_3D{"Texture3D"};
        inline constexpr AssetType MESH{"Mesh"};
        inline constexpr AssetType MATERIAL{"Material"};
        inline constexpr AssetType SHADER{"Shader"};
        inline constexpr AssetType AUDIO{"Audio"};
        inline constexpr AssetType FONT{"Font"};
        inline constexpr AssetType SCRIPT{"Script"};
        inline constexpr AssetType SCENE{"Scene"};
    }

    struct AssetMetadata {
        AssetHandle handle{0};
        AssetType type = Asset::NONE;
        std::filesystem::path filepath;
        std::filesystem::path compiled_filepath; // gbs -> gshader
        bool runtime_only{false};

        [[nodiscard]] bool valid() const { return handle != 0 && type.valid(); }
        [[nodiscard]] bool has_source() const { return !compiled_filepath.empty(); }
    };
}

template<>
struct std::hash<engine::AssetType> {
    std::size_t operator()(const engine::AssetType& type) const noexcept {
        return std::hash<uint64_t>{}(type.id);
    }
};
