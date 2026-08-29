//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "panel/ObjectInspector.hpp"

namespace editor {
    void ObjectInspector::init(std::shared_ptr<engine::Scene> scene, std::shared_ptr<engine::AssetManager> asset_manager) {
        scene_ = std::move(scene);
        asset_manager_ = std::move(asset_manager);
    }

    #ifdef ENABLE_IMGUI
    void ObjectInspector::render_imgui() {
        ImGui::Begin("Inspector");
        if (selected_entity_) {
            draw_component(selected_entity_);
        }
        ImGui::End();
    }

    void ObjectInspector::draw_component(engine::Entity entity) const {
        if (selected_entity_.has_component<engine::ActiveComponent>()) {
            auto& active_comp = entity.get_component<engine::ActiveComponent>();
            ImGui::Checkbox("##Active", &active_comp.active);
            ImGui::SameLine();
        }

        // --------------------
        // Label
        // --------------------
        if (entity.has_component<engine::LabelComponent>()) {
            auto& label = entity.get_component<engine::LabelComponent>();

            char buffer[256]{};
            std::strncpy(buffer, label.tag.c_str(), sizeof(buffer) - 1);

            if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
                label.tag = std::string(buffer);
            }
        }
        ImGui::Separator();

        // --------------------
        // Transform
        // --------------------
        if (entity.has_component<engine::TransformComponent>()) {
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                auto& transform = entity.get_component<engine::TransformComponent>();

                ImGui::DragFloat3("Position", glm::value_ptr(transform.position), 0.1f);
                ImGui::DragFloat3("Rotation", glm::value_ptr(transform.rotation), 1.0f);
                ImGui::DragFloat3("Scale", glm::value_ptr(transform.scale), 0.05f, 0.001f, 1000.0f);
            }
        }

        // --------------------
        // Mesh
        // --------------------
        if (entity.has_component<engine::MeshComponent>()) {
            if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen)) {
                auto& mesh_comp = entity.get_component<engine::MeshComponent>();
                const auto& current_meta = asset_manager_->get_metadata(mesh_comp.handle);

                std::string current_label = current_meta.is_valid() ? current_meta.filepath.filename().string() : "None / Select Mesh";

                if (ImGui::BeginCombo("Mesh Asset", current_label.c_str())) {
                    for (const auto& [handle, meta] : asset_manager_->get_registry()) {
                        if (meta.type != engine::AssetType::MESH) continue;
                        const bool is_selected = (mesh_comp.handle == handle);
                        std::string item_name = meta.filepath.filename().string();
                        if (ImGui::Selectable(item_name.c_str(), is_selected)) {
                            mesh_comp.handle = handle;
                        }
                        if (is_selected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                ImGui::TextDisabled("Handle UUID: %llu", static_cast<uint64_t>(mesh_comp.handle));
            }
        }

        // --------------------
        // Material
        // --------------------
        if (entity.has_component<engine::MaterialComponent>()) {
            if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
                auto& material = entity.get_component<engine::MaterialComponent>();
                ImGui::ColorEdit4("Color", glm::value_ptr(material.color));

                ImGui::SliderFloat("Roughness", &material.roughness, 0.0f, 1.0f);
                ImGui::SliderFloat("Metallic", &material.metallic, 0.0f, 1.0f);
                ImGui::Separator();
                const auto& current_tex_meta = asset_manager_->get_metadata(material.texture_handle);
                std::string tex_label = current_tex_meta.is_valid()
                                            ? current_tex_meta.filepath.filename().string()
                                            : "None / Select Texture";

                if (ImGui::BeginCombo("Texture", tex_label.c_str())) {
                    if (ImGui::Selectable("None", material.texture_handle == 0)) {
                        material.texture_handle = {0};
                    }

                    for (const auto& [handle, meta] : asset_manager_->get_registry()) {
                        if (meta.type != engine::AssetType::TEXTURE_2D) continue;

                        const bool is_selected = (material.texture_handle == handle);
                        std::string item_name = meta.filepath.filename().string();

                        if (ImGui::Selectable(item_name.c_str(), is_selected)) {
                            material.texture_handle = handle;
                        }
                        if (is_selected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM")) {
                        engine::AssetHandle dropped_handle = *static_cast<const engine::AssetHandle*>(payload->Data);
                        const auto& meta = asset_manager_->get_metadata(dropped_handle);

                        if (meta.is_valid() && meta.type == engine::AssetType::TEXTURE_2D) {
                            material.texture_handle = dropped_handle;
                        }
                    }
                    ImGui::EndDragDropTarget();
                }

                if (material.texture_handle != 0) {
                    ImGui::TextDisabled("Texture Handle: %llu", static_cast<uint64_t>(material.texture_handle));
                }
            }
        }

        // --------------------
        // Lighting
        // --------------------
        if (entity.has_component<engine::DirectionalLightComponent>()) {
            if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen)) {
                auto& dir_light = entity.get_component<engine::DirectionalLightComponent>();
                ImGui::DragFloat("Intensity", &dir_light.intensity, 0.01);
                ImGui::ColorEdit4("Color", glm::value_ptr(dir_light.color));
                bool todo_shadow_button = true;
                ImGui::Checkbox("Cast Shadows", &todo_shadow_button);
            }
        }
        if (entity.has_component<engine::PointLightComponent>()) {
            if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen)) {
                auto& point_light = entity.get_component<engine::PointLightComponent>();
                ImGui::DragFloat("Intensity", &point_light.intensity, 0.01);
                ImGui::ColorEdit4("Color", glm::value_ptr(point_light.color));
                ImGui::DragFloat("Radius", &point_light.radius);
                bool todo_shadow_button = true;
                ImGui::Checkbox("Cast Shadows", &todo_shadow_button);
            }
        }

        // --------------------
        // Box Collider
        // --------------------
        if (entity.has_component<engine::BoxColliderComponent>()) {
            if (ImGui::CollapsingHeader("Box Collider", ImGuiTreeNodeFlags_DefaultOpen)) {
                auto& collider = entity.get_component<engine::BoxColliderComponent>();

                ImGui::DragFloat3("Center", glm::value_ptr(collider.center), 0.05f);
                ImGui::DragFloat3("Size", glm::value_ptr(collider.size), 0.05f, 0.01f, 100.0f);
                ImGui::Checkbox("Is Trigger", &collider.is_trigger);
                ImGui::Checkbox("Show Hitbox", &collider.show_hitbox);

                if (entity.has_component<engine::MeshComponent>()) {
                    if (ImGui::Button("Fit to Mesh")) {
                        const auto& mesh_comp = entity.get_component<engine::MeshComponent>();
                        if (auto mesh = asset_manager_->get_asset<engine::Mesh>(mesh_comp.handle)) {
                            collider.size = mesh->local_bounds.max - mesh->local_bounds.min;
                            collider.center = (mesh->local_bounds.min + mesh->local_bounds.max) * 0.5f;
                        }
                    }
                }
            }
        }

        ImGui::Separator();
        ImGui::Spacing();

        // --------------------
        // Add Component Popup
        // --------------------
        if (ImGui::Button("+ Add Component")) {
            ImGui::OpenPopup("AddComponentPopup");
        }
        if (ImGui::BeginPopup("AddComponentPopup")) {
            if (!entity.has_component<engine::MaterialComponent>()) {
                if (ImGui::MenuItem("Material Component")) {
                    entity.add_component<engine::MaterialComponent>(glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});
                    ImGui::CloseCurrentPopup();
                }
            }
            if (!entity.has_component<engine::BoxColliderComponent>()) {
                if (ImGui::MenuItem("Box Collider")) {
                    auto& collider = entity.add_component<engine::BoxColliderComponent>();

                    if (entity.has_component<engine::MeshComponent>()) {
                        const auto& mesh_comp = entity.get_component<engine::MeshComponent>();
                        if (auto mesh = asset_manager_->get_asset<engine::Mesh>(mesh_comp.handle)) {
                            collider.size = mesh->local_bounds.max - mesh->local_bounds.min;
                            collider.center = (mesh->local_bounds.min + mesh->local_bounds.max) * 0.5f;
                        }
                    }
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndPopup();
        }
    }
    #endif
}
