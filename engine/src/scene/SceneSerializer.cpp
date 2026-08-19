//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "scene/SceneSerializer.hpp"

#include <fstream>
#include <nlohmann/json.hpp>
#include <render/mesh/MeshLoader.hpp>

#include "scene/ecs/Entity.hpp"

using json = nlohmann::json;

namespace engine {
    static json serialize_vec3(const glm::vec3 v) { return {v.x, v.y, v.z}; }
    static json serialize_vec4(const glm::vec4& v) { return {v.x, v.y, v.z, v.w}; }

    static glm::vec3 deserialize_vec3(const json& j) { return {j[0], j[1], j[2]}; }
    static glm::vec4 deserialize_vec4(const json& j) { return {j[0], j[1], j[2], j[3]}; }

    void SceneSerializer::serialize(const std::string& filepath) {
        json root;
        root["scenes"] = "Untitled";
        root["entities"] = json::array();

        auto view = scene_->get_registry().view<IDComponent>();
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
                entity_json["mesh"] = {
                    {"filepath", mc.path}
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
        }

        std::ofstream out(filepath);
        if (out.is_open()) {
            out << root.dump(4);
        }
    }

    bool SceneSerializer::deserialize(const std::string& filepath) {
        std::ifstream in(filepath);
        if (!in.is_open()) return false;

        json root;
        in >> root;

        scene_->get_registry().clear(); // Alte Szene leeren

        for (const auto& entity_json : root["entities"]) {
            std::string name = entity_json.value("label", "Entity");
            Entity entity = scene_->create_entity(name);

            if (entity_json.contains("id")) {
                entity.get_component<IDComponent>().id = UUID(entity_json["id"].get<uint64_t>());
            }
            if (entity_json.contains("transform")) {
                auto& tc = entity.get_component<TransformComponent>();
                auto& t_json = entity_json["transform"];
                tc.position = deserialize_vec3(t_json["position"]);
                tc.rotation = deserialize_vec3(t_json["rotation"]);
                tc.scale = deserialize_vec3(t_json["scale"]);
            }
            if (entity_json.contains("mesh")) {
                std::string path = entity_json["mesh"]["filepath"];
                // Mesh über deinen Loader laden und anhängen
                auto mesh = MeshLoader::load(path);
                entity.add_component<MeshComponent>(mesh, path);
            }

            if (entity_json.contains("camera")) {
                auto& cc = entity.add_component<CameraComponent>();
                auto& c_json = entity_json["camera"];
                cc.camera.set_fov(c_json["fov"]);
                cc.camera.set_near_clip(c_json["near_plane"]);
                cc.camera.set_far_clip(c_json["far_plane"]);
                cc.is_primary = c_json["is_primary"];
            }
            if (entity_json.contains("material")) {
                auto& mc = entity.add_component<MaterialComponent>();
                mc.color = deserialize_vec4(entity_json["material"]["color"]);
            }
            if (entity_json.contains("directional_light")) {
                auto& dlc = entity.add_component<DirectionalLightComponent>();
                auto& l_json = entity_json["directional_light"];
                dlc.color = deserialize_vec3(l_json["color"]);
                dlc.intensity = l_json["intensity"];
            }
            if (entity_json.contains("point_light")) {
                auto& plc = entity.add_component<PointLightComponent>();
                auto& l_json = entity_json["point_light"];
                plc.color = deserialize_vec3(l_json["color"]);
                plc.intensity = l_json["intensity"];
                plc.radius = l_json["radius"];
            }
        }

        return true;
    }
}
