//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include "core/UUID.hpp"

namespace engine {
    using AssetHandle = UUID;

    enum class AssetType : uint16_t {
        NONE = 0,
        TEXTURE_2D,
        TEXTURE_3D,
        MESH,
        MATERIAL,
        SHADER,
        AUDIO
    };

    inline std::string_view asset_type_to_string(const AssetType type) {
        switch (type) {
            case AssetType::TEXTURE_2D : return "Texture2d";
            case AssetType::MESH : return "Mesh";
            case AssetType::MATERIAL : return "Material";
            case AssetType::SHADER : return "Shader";
            case AssetType::AUDIO : return "Audio";
            default : return "None";
        }
    }

    inline AssetType string_to_asset_type(const std::string_view type_str) {
        if (type_str == "Texture2d") return AssetType::TEXTURE_2D;
        if (type_str == "Mesh") return AssetType::MESH;
        if (type_str == "Material") return AssetType::MATERIAL;
        if (type_str == "Shader") return AssetType::SHADER;
        if (type_str == "Audio") return AssetType::AUDIO;
        return AssetType::NONE;
    }

    struct AssetMetadata {
        AssetHandle handle{0};
        AssetType type = AssetType::NONE;
        std::filesystem::path filepath;

        [[nodiscard]] bool is_valid() const { return handle != 0 && type != AssetType::NONE; }
    };
}
