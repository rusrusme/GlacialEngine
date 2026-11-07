//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "project/scene/SceneSerializer.hpp"

#include <future>

#include "asset/AssetManager.hpp"
#include "core/Log.hpp"
#include "core/SystemContext.hpp"
#include "project/scene/SceneManager.hpp"
#include "project/scene/ecs/Entity.hpp"

using json = nlohmann::json;

namespace engine {
    void SceneSerializer::serialize(const std::string& filepath, SystemContext& context) {
        // MAIN THREAD: Build JSON in memory (fast, avoids data races)
        CORE_INFO("Serializing scene to '{0}'", filepath);

        auto scene = context.scenes.active_scene();
        if (!scene) {
            CORE_ERROR("Failed to serialize scene: Active scene is invalid");
            return;
        }

        std::string scene_name = std::filesystem::path(filepath).stem().string();
        scene->set_name(scene_name);
        scene->set_path(filepath);

        json root;
        root["scene"] = scene_name;
        root["path"] = filepath;
        root["entities"] = json::array();

        size_t entity_count = 0;
        auto view = scene->view<IDComponent>();
        for (auto entity_handle : view) {
            Entity entity{entity_handle, scene.get()};
            if (!entity) continue;
            root["entities"].push_back(serialize_entity(entity, context));
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

    bool SceneSerializer::deserialize(const std::string& filepath, SystemContext& context) {
        CORE_INFO("Deserializing scene from '{0}'", filepath);

        auto scene = context.scenes.active_scene();
        if (!scene) {
            CORE_ERROR("Failed to deserialize scene: Active scene is invalid");
            return false;
        }

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

        scene->clear();

        scene->set_path(filepath);
        std::string scene_name = root.value("scene", std::filesystem::path(filepath).stem().string());
        scene->set_name(scene_name);

        size_t loaded_count = 0;
        for (const auto& entity_json : root["entities"]) {
            deserialize_entity(entity_json, context);
            loaded_count++;
        }

        CORE_INFO("Successfully loaded scene '{0}' ({1} entities) from '{2}'", scene_name, loaded_count, filepath);
        return true;
    }

    nlohmann::json SceneSerializer::serialize_entity(Entity entity, SystemContext& context) {
        if (!entity) return {};

        auto& assets = context.assets;
        json entity_json;

        // ID & Label
        if (entity.has_component<IDComponent>()) {
            entity_json["id"] = static_cast<uint64_t>(entity.get_component<IDComponent>().id);
        }
        if (entity.has_component<LabelComponent>()) {
            entity_json["label"] = entity.get_component<LabelComponent>().tag;
        }
        entity_json["active"] = entity.is_active();

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
            std::string path = assets.path_from_asset_handle(mc.handle).string();
            entity_json["mesh"] = {
                {"handle", static_cast<uint64_t>(mc.handle)},
                {"path", path}
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

        return entity_json;
    }

    Entity SceneSerializer::deserialize_entity(const nlohmann::json& entity_json, SystemContext& context) {
        auto scene = context.scenes.active_scene();
        auto& assets = context.assets;
        if (!scene) {
            CORE_ERROR("Failed to deserialize entity: Active scene is invalid");
            return {};
        }

        if (!entity_json.contains("id")) {
            CORE_ERROR("Failed to deserialize entity: Missing UUID identifier");
            return {};
        }

        const UUID uuid(entity_json["id"].get<uint64_t>());
        const std::string label = entity_json.value("label", "Entity");

        Entity entity = scene->create_entity_with_uuid(uuid, label);
        entity.set_active(entity_json.value("active", true));

        // Transform
        if (entity_json.contains("transform")) {
            auto& tc = entity.get_component<TransformComponent>();
            const auto& t = entity_json["transform"];
            if (t.contains("position")) tc.position = deserialize_vec3(t["position"]);
            if (t.contains("rotation")) tc.rotation = deserialize_vec3(t["rotation"]);
            if (t.contains("scale")) tc.scale = deserialize_vec3(t["scale"]);
        }

        // Mesh
        if (entity_json.contains("mesh")) {
            const auto& m = entity_json["mesh"];
            AssetHandle handle{0};
            if (m.contains("handle")) {
                handle = AssetHandle(m["handle"].get<uint64_t>());
            }
            if (!assets.valid_asset(handle) && m.contains("path")) {
                handle = assets.asset_handle_from_path(m["path"].get<std::string>());
            }
            entity.add_component<MeshComponent>(handle);
        }

        // Material
        if (entity_json.contains("material")) {
            const auto& m = entity_json["material"];
            glm::vec4 color{1.0f};
            if (m.is_object() && m.contains("color")) {
                color = deserialize_vec4(m["color"]);
            }
            else if (m.is_array()) { // Backward compatibility
                color = deserialize_vec4(m);
            }
            entity.add_component<MaterialComponent>(color);
        }

        // Directional Light
        if (entity_json.contains("directional_light")) {
            const auto& l = entity_json["directional_light"];
            entity.add_component<DirectionalLightComponent>(
                deserialize_vec3(l["color"]),
                l.value("intensity", 1.0f)
            );
        }

        // Point Light
        if (entity_json.contains("point_light")) {
            const auto& l = entity_json["point_light"];
            entity.add_component<PointLightComponent>(
                deserialize_vec3(l["color"]),
                l.value("intensity", 1.0f),
                l.value("radius", 10.0f)
            );
        }

        return entity;
    }

    nlohmann::json SceneSerializer::serialize_vec3(const glm::vec3& v) { return {v.x, v.y, v.z}; }

    nlohmann::json SceneSerializer::serialize_vec4(const glm::vec4& v) { return {v.x, v.y, v.z, v.w}; }

    glm::vec3 SceneSerializer::deserialize_vec3(const nlohmann::json& j) {
        return {j.at(0).get<float>(), j.at(1).get<float>(), j.at(2).get<float>()};
    }

    glm::vec4 SceneSerializer::deserialize_vec4(const nlohmann::json& j) {
        return {j.at(0).get<float>(), j.at(1).get<float>(), j.at(2).get<float>(), j.at(3).get<float>()};
    }
}
