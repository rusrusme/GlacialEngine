//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "ui/ObjectInspector.hpp"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace editor {
    void ObjectInspector::init(std::shared_ptr<engine::Scene> scene, std::shared_ptr<engine::AssetManager> asset_manager) {
        scene_ = std::move(scene);
        asset_manager_ = std::move(asset_manager);
    }

    void ObjectInspector::render_ui() {
        ImGui::Begin("Object Inspector");
        if (selected_entity_) {
            draw_component(selected_entity_);
        }
        else {
            ImGui::TextDisabled("Nothing selected");
        }
        ImGui::End();
    }

    void ObjectInspector::draw_component(engine::Entity entity) {
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
        // Mesh (Nutzung von AssetHandle)
        // --------------------
        if (entity.has_component<engine::MeshComponent>()) {
            if (ImGui::CollapsingHeader("Mesh Component", ImGuiTreeNodeFlags_DefaultOpen)) {
                auto& mesh_comp = entity.get_component<engine::MeshComponent>();

                // Metadaten zum Handle aus der Asset-Registry abfragen
                const auto& metadata = asset_manager_->get_metadata(mesh_comp.handle);

                std::string asset_name = metadata.is_valid()
                                             ? metadata.filepath.filename().string()
                                             : "None / Invalid Handle";

                ImGui::Text("Mesh Asset: %s", asset_name.c_str());
                ImGui::Text("Handle UUID: %llu", static_cast<uint64_t>(mesh_comp.handle));
            }
        }

        // --------------------
        // Material
        // --------------------
        if (entity.has_component<engine::MaterialComponent>()) {
            if (ImGui::CollapsingHeader("Material Component", ImGuiTreeNodeFlags_DefaultOpen)) {
                auto& material = entity.get_component<engine::MaterialComponent>();
                ImGui::ColorEdit4("Color", glm::value_ptr(material.color));
            }
        }

        ImGui::Separator();
        ImGui::Spacing();

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
            ImGui::EndPopup();
        }
    }
}
