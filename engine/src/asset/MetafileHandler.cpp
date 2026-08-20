//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "asset/MetafileHandler.hpp"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

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
                    return metadata;
                }
                catch (const std::exception& e) {
                    std::cerr << "Engine warning: Failed to parse meta file: " << meta_path << " (" << e.what() << ")\n";
                }
            }
        }
        metadata.handle = AssetHandle();

        json root;
        root["asset"] = {
            {"handle", static_cast<uint64_t>(metadata.handle)},
            {"type", std::string(asset_type_to_string(metadata.type))}
        };

        std::ofstream out(meta_path);
        if (out.is_open()) {
            out << root.dump(4);
        }
        return metadata;
    }

    AssetType MetaFileHandler::deduce_asset_type(const std::filesystem::path& filepath) {
        const std::string ext = filepath.extension().string();

        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga") {
            return AssetType::TEXTURE_2D;
        }
        if (ext == ".glb" || ext == ".gltf" || ext == ".obj") {
            return AssetType::MESH;
        }
        if (ext == ".glsl" || ext == ".vert" || ext == ".frag") {
            return AssetType::SHADER;
        }
        if (ext == ".mat") {
            return AssetType::MATERIAL;
        }
        return AssetType::NONE;
    }
}
