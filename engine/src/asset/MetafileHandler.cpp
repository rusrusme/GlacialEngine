//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "MetafileHandler.hpp"

#include <fstream>
#include <nlohmann/json.hpp>

#include "core/Log.hpp"

using json = nlohmann::json;

namespace engine {
    void MetafileHandler::register_defaults() {
        if (defaults_registered) return;
        defaults_registered = true;

        register_extension(".png", Asset::TEXTURE_2D, "Texture2D");
        register_extension(".jpg", Asset::TEXTURE_2D, "Texture2D");
        register_extension(".jpeg", Asset::TEXTURE_2D, "Texture2D");

        register_extension(".glb", Asset::MESH, "Mesh");
        register_extension(".gltf", Asset::MESH, "Mesh");

        register_extension(".gshader", Asset::SHADER, "Shader");
        register_extension(".gsb", Asset::SHADER, "Shader");

        register_extension(".mat", Asset::MATERIAL, "Material");

        register_extension(".wav", Asset::AUDIO, "Audio");
        register_extension(".mp3", Asset::AUDIO, "Audio");
        register_extension(".ogg", Asset::AUDIO, "Audio");
        register_extension(".flac", Asset::AUDIO, "Audio");

        register_extension(".ttf", Asset::FONT, "Font");

        register_extension(".gscript", Asset::SCRIPT, "Script");
        register_extension(".gscene", Asset::SCENE, "Scene");
    }

    void MetafileHandler::register_extension(std::string_view extension, AssetType type, std::string_view type_name) {
        std::string key = util::to_lowercase(std::string(extension));
        if (!key.empty() && key[0] != '.') {
            key.insert(key.begin(), '.');
        }
        extensions_[std::move(key)] = ExtensionEntry{.type = type, .name = std::string(type_name)};
    }

    bool MetafileHandler::is_asset_file(const std::filesystem::path& filepath) {
        if (filepath.extension() == ".meta") {
            return false;
        }
        if (find_extension(filepath) != nullptr) {
            return true;
        }
        std::error_code ec;
        return std::filesystem::exists(filepath.string() + ".meta", ec);
    }

    AssetMetadata MetafileHandler::resolve(const std::filesystem::path& asset_path, bool generate_missing) {
        AssetMetadata meta;
        meta.filepath = asset_path.lexically_normal();

        const ExtensionEntry* ext_entry = find_extension(meta.filepath);
        if (ext_entry) {
            meta.type = ext_entry->type;
        }

        const std::filesystem::path meta_path = meta.filepath.string() + ".meta";
        bool needs_save = false;

        if (std::filesystem::exists(meta_path)) {
            std::ifstream in(meta_path);
            if (in.is_open()) {
                try {
                    json root;
                    in >> root;
                    meta.handle = AssetHandle(root["asset"]["handle"].get<uint64_t>());

                    if (root["asset"].contains("type")) {
                        meta.type = AssetType(root["asset"]["type"].get<std::string>());
                    }

                    if (root["asset"].contains("source")) {
                        meta.shader_source_filepath = root["asset"]["source"].get<std::string>();
                    }
                    else if (util::to_lowercase(meta.filepath.extension().string()) == ".gsb") {
                        meta.shader_source_filepath = find_shader_source(meta.filepath);
                        if (!meta.shader_source_filepath.empty()) {
                            needs_save = true;
                        }
                    }

                    if (!needs_save || !generate_missing) {
                        return meta;
                    }
                }
                catch (const std::exception& e) {
                    CORE_ERROR("Failed to parse metafile: {0} ({1})", meta_path.string(), e.what());
                }
            }
        }

        if (!meta.type.valid() || !generate_missing) {
            return meta;
        }

        if (meta.handle == 0) {
            meta.handle = AssetHandle();
            if (util::to_lowercase(meta.filepath.extension().string()) == ".gsb") {
                meta.shader_source_filepath = find_shader_source(meta.filepath);
            }
        }

        const std::string type_name = ext_entry ? ext_entry->name : "Unknown";

        json root;
        root["asset"] = {
            {"handle", static_cast<uint64_t>(meta.handle)},
            {"type", type_name}
        };

        if (!meta.shader_source_filepath.empty()) {
            root["asset"]["source"] = meta.shader_source_filepath.generic_string();
        }

        if (std::ofstream out(meta_path); out.is_open()) {
            out << root.dump(4);
            CORE_TRACE("Created/Updated metadata for {0} asset from {1}",
                       util::to_lowercase(type_name), meta.filepath.string());
        }
        else {
            CORE_ERROR("Failed to write metadata file to disk: '{0}'", meta_path.string());
        }

        return meta;
    }

    const MetafileHandler::ExtensionEntry* MetafileHandler::find_extension(const std::filesystem::path& path) {
        if (!defaults_registered) {
            register_defaults();
        }
        const std::string ext = util::to_lowercase(path.extension().string());
        if (const auto it = extensions_.find(ext); it != extensions_.end()) {
            return &it->second;
        }
        return nullptr;
    }

    std::filesystem::path MetafileHandler::find_shader_source(const std::filesystem::path& compiled_path) {
        std::filesystem::path source_path = compiled_path;
        if (util::to_lowercase(source_path.extension().string()) == ".gsb") {
            source_path.replace_extension(".gshader");
        }

        std::string path_str = source_path.generic_string();
        constexpr std::string_view compiled_segment = "compiled/";
        if (const size_t pos = path_str.find(compiled_segment);
            pos != std::string::npos && (pos == 0 || path_str[pos - 1] == '/')) {
            path_str.erase(pos, compiled_segment.length());

            if (std::filesystem::path candidate(path_str); std::filesystem::exists(candidate)) {
                return candidate;
            }
        }

        if (std::filesystem::exists(source_path)) {
            return source_path;
        }

        return {};
    }
}
