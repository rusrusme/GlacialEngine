//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "ui/Hierarchy.hpp"

#include <imgui.h>
#include <core/Log.hpp>
#include <input/Input.hpp>
#include <input/KeyCodes.hpp>
#include <render/mesh/Mesh.hpp>
#include <render/mesh/PrimitiveMesh.hpp>
#include <scene/ecs/Entity.hpp>

namespace editor {
    engine::Entity Hierarchy::add_entity(const std::string& name) {
        const engine::Entity entity = scene_->create_entity(name);
        selected_entity_ = entity;
        CLIENT_INFO("Created entity - {0}", name.c_str());
        return entity;
    }

    void Hierarchy::render_ui() {
        engine::Entity entity_to_destroy{};

        ImGui::Begin("Hierarchy");
        if (ImGui::Button("+")) {
            ImGui::OpenPopup("AddEntityMenu");
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputText("##NameInput", name_buffer_, sizeof(name_buffer_));
        if (ImGui::BeginPopup("AddEntityMenu")) {
            if (ImGui::MenuItem("Entity")) {
                const std::string name = name_buffer_[0] != '\0' ? name_buffer_ : "Entity";
                scene_->create_entity(name);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Plane")) {
                const std::string name = name_buffer_[0] != '\0' ? name_buffer_ : "Plane";
                auto entity = scene_->create_entity(name);
                engine::Mesh mesh = engine::PrimitiveMesh::create_plane();
                entity.add_component<engine::MeshComponent>(mesh, "primitive://plane");
                entity.add_component<engine::MaterialComponent>(glm::vec4{1.0f, 0.5f, 0.2f, 1.0f});
            }
            if (ImGui::MenuItem("Circle")) {
                const std::string name = name_buffer_[0] != '\0' ? name_buffer_ : "Circle";
                auto entity = scene_->create_entity(name);
                engine::Mesh mesh = engine::PrimitiveMesh::create_circle();
                entity.add_component<engine::MeshComponent>(mesh, "primitive://circle");
                entity.add_component<engine::MaterialComponent>(glm::vec4{1.0f, 0.5f, 0.2f, 1.0f});
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Cube")) {
                const std::string name = name_buffer_[0] != '\0' ? name_buffer_ : "Cube";
                auto entity = scene_->create_entity(name);
                engine::Mesh mesh = engine::PrimitiveMesh::create_cube();
                entity.add_component<engine::MeshComponent>(mesh, "primitive://cube");
                entity.add_component<engine::MaterialComponent>(glm::vec4{1.0f, 0.5f, 0.2f, 1.0f});
            }
            if (ImGui::MenuItem("Sphere")) {
                const std::string name = name_buffer_[0] != '\0' ? name_buffer_ : "Sphere";
                auto entity = scene_->create_entity(name);
                engine::Mesh mesh = engine::PrimitiveMesh::create_uv_sphere();
                entity.add_component<engine::MeshComponent>(mesh, "primitive://sphere");
                entity.add_component<engine::MaterialComponent>(glm::vec4{1.0f, 0.5f, 0.2f, 1.0f});
            }
            if (ImGui::MenuItem("Cylinder")) {
                const std::string name = name_buffer_[0] != '\0' ? name_buffer_ : "Cylinder";
                auto entity = scene_->create_entity(name);
                engine::Mesh mesh = engine::PrimitiveMesh::create_cylinder();
                entity.add_component<engine::MeshComponent>(mesh, "primitive://cylinder");
                entity.add_component<engine::MaterialComponent>(glm::vec4{1.0f, 0.5f, 0.2f, 1.0f});
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Camera")) {
                const std::string name = name_buffer_[0] != '\0' ? name_buffer_ : "Camera";
                auto entity = scene_->create_entity(name);
                entity.add_component<engine::CameraComponent>();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Directional Light")) {
                const std::string name = name_buffer_[0] != '\0' ? name_buffer_ : "Light";
                auto entity = scene_->create_entity(name);
                auto& transform = entity.get_component<engine::TransformComponent>();
                transform.rotation = glm::vec3(-90.0f, 135.0f, -45.0f);
                transform.position = glm::vec3(10.0f, 10.0f, 0.0f);
                entity.add_component<engine::DirectionalLightComponent>(
                    glm::vec3(1.0f, 0.95f, 0.8f),
                    1.2f
                );
            }
            ImGui::EndPopup();
        }
        ImGui::Separator();
        auto view = scene_->get_registry().view<engine::LabelComponent>();
        for (auto raw_entity : view) {
            engine::Entity entity{raw_entity, scene_.get()};
            const auto& label = entity.get_component<engine::LabelComponent>();
            ImGui::PushID(entity.uuid());

            const bool is_selected = selected_entity_ == raw_entity;
            const ImGuiTreeNodeFlags flags = (is_selected ? ImGuiTreeNodeFlags_Selected : 0)
                | ImGuiTreeNodeFlags_OpenOnArrow
                | ImGuiTreeNodeFlags_SpanAvailWidth;
            const bool opened = ImGui::TreeNodeEx(label.tag.c_str(), flags);

            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
                selected_entity_ = entity;
            }
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Delete")) {
                    entity_to_destroy = entity;
                }
                ImGui::EndPopup();
            }
            if (opened) {
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        if (entity_to_destroy) {
            if (selected_entity_ == entity_to_destroy) {
                CLIENT_INFO("Deleted entity - {0}", entity_to_destroy.name().c_str());
                selected_entity_ = {};
            }
            scene_->destroy_entity(entity_to_destroy);
        }

        if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered()) {
            selected_entity_ = {};
        }
        if (ImGui::IsWindowFocused() && engine::Input::is_key_pressed_once(engine::Key::ESCAPE)) {
            selected_entity_ = {};
        }

        ImGui::End();
    }
}
