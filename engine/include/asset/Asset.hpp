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

    enum class Asset : uint16_t {
        NONE = 0,
        TEXTURE_2D,
        TEXTURE_3D,
        MESH,
        MATERIAL,
        SHADER,
        AUDIO,
        FONT,
        SCRIPT,
        SCENE
    };

    constexpr std::string_view asset_type_to_string(const Asset type) {
        switch (type) {
            case Asset::TEXTURE_2D : return "Texture2D";
            case Asset::TEXTURE_3D : return "Texture3D";
            case Asset::MESH : return "Mesh";
            case Asset::MATERIAL : return "Material";
            case Asset::SHADER : return "Shader";
            case Asset::AUDIO : return "Audio";
            case Asset::FONT : return "Font";
            case Asset::SCRIPT : return "Script";
            case Asset::SCENE : return "Scene";
            case Asset::NONE :
            default : return "None";
        }
    }

    constexpr Asset string_to_asset_type(const std::string_view type_str) {
        if (type_str == "Texture2D") return Asset::TEXTURE_2D;
        if (type_str == "Texture3D") return Asset::TEXTURE_3D;
        if (type_str == "Mesh") return Asset::MESH;
        if (type_str == "Material") return Asset::MATERIAL;
        if (type_str == "Shader") return Asset::SHADER;
        if (type_str == "Audio") return Asset::AUDIO;
        if (type_str == "Font") return Asset::FONT;
        if (type_str == "Script") return Asset::SCRIPT;
        if (type_str == "Scene") return Asset::SCENE;
        return Asset::NONE;
    }

    struct AssetMetadata {
        AssetHandle handle{0};
        Asset type = Asset::NONE;
        std::filesystem::path filepath;
        std::filesystem::path source_filepath; // gbs -> gshader

        [[nodiscard]] bool valid() const { return handle != 0 && type != Asset::NONE; }
        [[nodiscard]] bool has_source() const { return !source_filepath.empty(); }
    };
}
