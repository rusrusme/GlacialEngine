//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "scene/SceneSerializer.hpp"

#include <future>

#include "asset/AssetManager.hpp"
#include "core/Log.hpp"
#include "core/SystemContext.hpp"
#include "scene/ecs/Entity.hpp"

using json = nlohmann::json;

namespace engine {
    static json serialize_vec3(const glm::vec3 v) { return {v.x, v.y, v.z}; }
    static json serialize_vec4(const glm::vec4& v) { return {v.x, v.y, v.z, v.w}; }

    static glm::vec3 deserialize_vec3(const json& j) { return {j[0], j[1], j[2]}; }
    static glm::vec4 deserialize_vec4(const json& j) { return {j[0], j[1], j[2], j[3]}; }

    void SceneSerializer::serialize(const std::string& filepath, AssetManager& asset_manager) {
        // MAIN THREAD: Build JSON in memory (fast, avoids data races)
        CORE_INFO("Serializing scene to '{0}'", filepath);

        std::string scene_name = std::filesystem::path(filepath).stem().string();
        scene_->set_name(scene_name);
        scene_->set_path(filepath);

        json root;
        root["scene"] = scene_name;
        root["path"] = filepath;
        root["entities"] = json::array();

        size_t entity_count = 0;
        auto view = scene_->view<IDComponent>();
        for (auto entity_handle : view) {
            Entity entity{entity_handle, scene_.get()};
            if (!entity) continue;

            json entity_json;

            // IDComponent & LabelComponent
            if (entity.has_component<IDComponent>()) {
                entity_json["id"] = static_cast<uint64_t>(entity.get_component<IDComponent>().id);
            }
            if (entity.has_component<LabelComponent>()) {
                entity_json["label"] = entity.get_component<LabelComponent>().tag;
            }
            if (entity.has_component<ActiveComponent>()) {
                entity_json["active"] = entity.get_component<ActiveComponent>().active;
            }
            // TransformComponent
            if (entity.has_component<TransformComponent>()) {
                auto& transform = entity.get_component<TransformComponent>();
                entity_json["transform"] = {
                    {"position", serialize_vec3(transform.position)},
                    {"rotation", serialize_vec3(transform.rotation)},
                    {"scale", serialize_vec3(transform.scale)}
                };
            }
            // MeshComponent
            if (entity.has_component<MeshComponent>()) {
                auto& mc = entity.get_component<MeshComponent>();
                std::string path = asset_manager.get_path_from_handle(mc.handle).string();
                entity_json["mesh"] = {
                    {"handle", static_cast<uint64_t>(mc.handle)},
                    {"path", path}
                };
            }
            // CameraComponent
            if (entity.has_component<CameraComponent>()) {
                auto& cc = entity.get_component<CameraComponent>();
                entity_json["camera"] = {
                    {"fov", cc.camera.get_fov()},
                    {"near_plane", cc.camera.get_near_clip()},
                    {"far_plane", cc.camera.get_far_clip()},
                    {"is_primary", cc.is_primary}
                };
            }
            // MaterialComponent
            if (entity.has_component<MaterialComponent>()) {
                auto& mc = entity.get_component<MaterialComponent>();
                entity_json["material"] = {
                    {"color", serialize_vec4(mc.color)}
                };
            }
            // DirectionalLightComponent
            if (entity.has_component<DirectionalLightComponent>()) {
                auto& dlc = entity.get_component<DirectionalLightComponent>();
                entity_json["directional_light"] = {
                    {"color", serialize_vec3(dlc.color)},
                    {"intensity", dlc.intensity}
                };
            }
            // PointLightComponent
            if (entity.has_component<PointLightComponent>()) {
                auto& plc = entity.get_component<PointLightComponent>();
                entity_json["point_light"] = {
                    {"color", serialize_vec3(plc.color)},
                    {"intensity", plc.intensity},
                    {"radius", plc.radius}
                };
            }
            root["entities"].push_back(entity_json);
            entity_count++;
        }

        // BACKGROUND THREAD: Offload disk I/O
        std::thread([filepath, root = std::move(root), entity_count]() {
            std::ofstream out(filepath);
            if (!out.is_open()) {
                CORE_ERROR("Failed to serialize scene: Could not open file for writing at '{0}'", filepath);
                return;
            }
            out << root.dump(4);
            CORE_INFO("Successfully saved scene '{0}' ({1} entities) [Background Thread]", filepath, entity_count);
        }).detach();
    }

    bool SceneSerializer::deserialize(const std::string& filepath, const SystemContext& context) {
        CORE_INFO("Deserializing scene from '{0}'", filepath);
        auto& asset_manager = context.assets;

        std::ifstream in(filepath);
        if (!in.is_open()) {
            CORE_ERROR("Failed to deserialize scene: Could not open file at '{0}'", filepath);
            return false;
        }

        json root;
        try {
            in >> root;
        }
        catch (const json::parse_error& e) {
            CORE_ERROR("Failed to parse scene file '{0}': {1}", filepath, e.what());
            return false;
        }

        if (!root.contains("entities") || !root["entities"].is_array()) {
            CORE_ERROR("Failed to deserialize scene '{0}': Missing valid 'entities' array", filepath);
            return false;
        }

        scene_->clear(context);

        scene_->set_path(filepath);
        std::string scene_name = root.value("scene", std::filesystem::path(filepath).stem().string());
        scene_->set_name(scene_name);

        size_t loaded_count = 0;

        for (const auto& entity_json : root["entities"]) {
            std::string name = entity_json.value("label", "Entity");

            const UUID uuid = entity_json.contains("id")
                                  ? UUID(entity_json["id"].get<uint64_t>())
                                  : UUID();

            Entity entity = scene_->create_entity_with_uuid(uuid, name);

            if (entity_json.contains("active")) {
                auto& ac = entity.get_component<ActiveComponent>();
                ac.active = entity_json["active"].get<bool>();
            }
            // Transform
            if (entity_json.contains("transform")) {
                auto& tc = entity.get_component<TransformComponent>();
                auto& t_json = entity_json["transform"];
                tc.position = deserialize_vec3(t_json["position"]);
                tc.rotation = deserialize_vec3(t_json["rotation"]);
                tc.scale = deserialize_vec3(t_json["scale"]);
            }
            // Mesh
            if (entity_json.contains("mesh")) {
                auto& m_json = entity_json["mesh"];
                AssetHandle handle{0};

                if (m_json.contains("handle")) {
                    handle = AssetHandle(m_json["handle"].get<uint64_t>());
                }
                // Asset recovery via path
                if (!asset_manager.is_asset_handle_valid(handle) && m_json.contains("path")) {
                    const std::string path = m_json["path"].get<std::string>();
                    AssetHandle resolved_handle = asset_manager.get_handle_from_path(path);

                    if (asset_manager.is_asset_handle_valid(resolved_handle)) {
                        CORE_WARN("Handle mismatch for entity '{0}': Stored handle: [{1}] Recovered handle [{2}] via path '{3}'",
                                  name, static_cast<uint64_t>(handle), static_cast<uint64_t>(resolved_handle), path);
                        handle = resolved_handle;
                    }
                    else {
                        CORE_ERROR("Failed to resolve mesh asset for entity '{0}': path '{1}' not found in asset manager", name, path);
                    }
                }
                entity.add_component<MeshComponent>(handle);
            }
            // 5. CameraComponent
            if (entity_json.contains("camera")) {
                auto& cc = entity.add_component<CameraComponent>();
                auto& c_json = entity_json["camera"];
                cc.camera.set_fov(c_json["fov"]);
                cc.camera.set_near_clip(c_json["near_plane"]);
                cc.camera.set_far_clip(c_json["far_plane"]);
                cc.is_primary = c_json["is_primary"];
            }
            // 6. MaterialComponent
            if (entity_json.contains("material")) {
                auto& mc = entity.add_component<MaterialComponent>();
                mc.color = deserialize_vec4(entity_json["material"]["color"]);
            }
            // 7. DirectionalLightComponent
            if (entity_json.contains("directional_light")) {
                auto& dlc = entity.add_component<DirectionalLightComponent>();
                auto& l_json = entity_json["directional_light"];
                dlc.color = deserialize_vec3(l_json["color"]);
                dlc.intensity = l_json["intensity"];
            }
            // 8. PointLightComponent
            if (entity_json.contains("point_light")) {
                auto& plc = entity.add_component<PointLightComponent>();
                auto& l_json = entity_json["point_light"];
                plc.color = deserialize_vec3(l_json["color"]);
                plc.intensity = l_json["intensity"];
                plc.radius = l_json["radius"];
            }
            loaded_count++;
        }
        CORE_INFO("Successfully loaded scene '{0}' ({1} entities) from '{2}'", scene_name, loaded_count, filepath);
        return true;
    }
}
