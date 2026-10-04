//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "asset/MetafileHandler.hpp"

#include "core/Log.hpp"

using json = nlohmann::json;

namespace engine {
    AssetMetadata MetaFileHandler::get_or_create_metadata(const std::filesystem::path& asset_path) {
        AssetMetadata metadata;
        metadata.filepath = asset_path;
        metadata.type = deduce_asset_type(asset_path);

        if (metadata.type == AssetType::NONE) {
            return metadata;
        }
        const std::filesystem::path meta_path = asset_path.string() + ".meta";

        if (std::filesystem::exists(meta_path)) {
            std::ifstream in(meta_path);
            if (in.is_open()) {
                try {
                    json root;
                    in >> root;
                    metadata.handle = AssetHandle(root["asset"]["handle"].get<uint64_t>());
                    CORE_TRACE("Loaded existing metadata for '{0}' [Handle: {1}]",
                               asset_path.string(), static_cast<uint64_t>(metadata.handle));
                    return metadata;
                }
                catch (const std::exception& e) {
                    CORE_ERROR("Failed to parse metafile: {0} ({1})", meta_path.string(), e.what());
                }
            }
        }
        metadata.handle = AssetHandle();

        json root;
        auto handle = static_cast<uint64_t>(metadata.handle);
        std::string type(asset_type_to_string(metadata.type));
        root["asset"] = {
            {"handle", handle},
            {"type", type}
        };

        if (std::ofstream out(meta_path); out.is_open()) {
            out << root.dump(4);
            CORE_TRACE("Created metadata for {0} asset from {1}", util::to_lowercase(type), metadata.filepath.string());
        }
        else {
            CORE_ERROR("Failed to write metadata file to disk: '{0}'", meta_path.string());
        }
        return metadata;
    }

    AssetType MetaFileHandler::deduce_asset_type(const std::filesystem::path& filepath) {
        const std::string ext = util::to_lowercase(filepath.extension().string());

        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga") {
            return AssetType::TEXTURE_2D;
        }
        if (ext == ".glb" || ext == ".gltf" || ext == ".obj") {
            return AssetType::MESH;
        }
        if (ext == ".vert" || ext == ".frag" || ext == ".geom" || ext == ".glsl" || ext == ".comp") {
            return AssetType::SHADER;
        }
        if (ext == ".bsh" || ext == ".bshb") {
            return AssetType::SHADER;
        }
        if (ext == ".mat") {
            return AssetType::MATERIAL;
        }
        if (ext == ".wav" || ext == ".mp3" || ext == ".ogg" || ext == ".flac") {
            return AssetType::AUDIO;
        }
        if (ext == ".ttf") {
            return AssetType::FONT;
        }
        return AssetType::NONE;
    }
}
