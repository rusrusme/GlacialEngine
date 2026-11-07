//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <nlohmann/json.hpp>

#include "asset/AssetManager.hpp"
#include "core/SystemContext.hpp"
#include "project/scene/Scene.hpp"
#include "project/scene/SceneManager.hpp"
#include "project/scene/ecs/Entity.hpp"

namespace editor {
    using namespace engine;
    using json = nlohmann::json;

    struct EntitySnapshot {
        static json serialize(Entity entity, SystemContext& context) {
            auto& asset_manager = context.assets;

            json data;
            data["id"] = static_cast<uint64_t>(entity.get_component<IDComponent>().id);
            data["label"] = entity.get_component<LabelComponent>().tag;
            data["active"] = entity.is_active();

            if (entity.has_component<TransformComponent>()) {
                auto& tc = entity.get_component<TransformComponent>();
                data["transform"] = {
                    {"pos", {tc.position.x, tc.position.y, tc.position.z}},
                    {"rot", {tc.rotation.x, tc.rotation.y, tc.rotation.z}},
                    {"scale", {tc.scale.x, tc.scale.y, tc.scale.z}}
                };
            }
            if (entity.has_component<MeshComponent>()) {
                auto& mc = entity.get_component<MeshComponent>();
                data["mesh"] = {
                    {"handle", static_cast<uint64_t>(mc.handle)},
                    {"path", asset_manager.path_from_asset_handle(mc.handle).string()}
                };
            }
            if (entity.has_component<MaterialComponent>()) {
                auto& mc = entity.get_component<MaterialComponent>();
                data["material"] = {mc.color.r, mc.color.g, mc.color.b, mc.color.a};
            }
            if (entity.has_component<DirectionalLightComponent>()) {
                auto& dlc = entity.get_component<DirectionalLightComponent>();
                data["dir_light"] = {
                    {"color", {dlc.color.r, dlc.color.g, dlc.color.b}},
                    {"intensity", dlc.intensity}
                };
            }
            if (entity.has_component<PointLightComponent>()) {
                auto& plc = entity.get_component<PointLightComponent>();
                data["point_light"] = {
                    {"color", {plc.color.r, plc.color.g, plc.color.b}},
                    {"intensity", plc.intensity},
                    {"radius", plc.radius}
                };
            }
            return data;
        }

        static Entity deserialize(const json& data, SystemContext& context) {
            auto scene = context.scenes.active_scene();
            if (!scene) {
                CORE_ERROR("Failed to deserialize entity: Active scene is invalid");
                return {};
            }

            UUID uuid(data["id"].get<uint64_t>());
            std::string label = data.value("label", "Entity");

            Entity entity = scene->create_entity_with_uuid(uuid, label);
            entity.set_active(data.value("active", true));

            if (data.contains("transform")) {
                auto& tc = entity.get_component<TransformComponent>();
                auto& t = data["transform"];
                tc.position = {t["pos"][0], t["pos"][1], t["pos"][2]};
                tc.rotation = {t["rot"][0], t["rot"][1], t["rot"][2]};
                tc.scale    = {t["scale"][0], t["scale"][1], t["scale"][2]};
            }
            if (data.contains("mesh")) {
                AssetHandle handle(data["mesh"]["handle"].get<uint64_t>());
                if (!context.assets.valid_asset(handle) && data["mesh"].contains("path")) {
                    handle = context.assets.asset_handle_from_path(data["mesh"]["path"].get<std::string>());
                }
                entity.add_component<MeshComponent>(handle);
            }
            if (data.contains("material")) {
                auto& m = data["material"];
                entity.add_component<MaterialComponent>(glm::vec4{m[0], m[1], m[2], m[3]});
            }
            if (data.contains("dir_light")) {
                auto& l = data["dir_light"];
                entity.add_component<DirectionalLightComponent>(
                    glm::vec3{l["color"][0], l["color"][1], l["color"][2]},
                    l["intensity"].get<float>()
                );
            }
            if (data.contains("point_light")) {
                auto& l = data["point_light"];
                entity.add_component<PointLightComponent>(
                    glm::vec3{l["color"][0], l["color"][1], l["color"][2]},
                    l["intensity"].get<float>()
                );
            }
            return entity;
        }
    };
}