//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "panel/Hierarchy.hpp"

#include "panel/EntityIcons.hpp"

namespace editor {
    void Hierarchy::init(std::shared_ptr<engine::Scene> scene, std::shared_ptr<engine::AssetManager> asset_manager) {
        scene_ = std::move(scene);
        asset_manager_ = std::move(asset_manager);
    }

    engine::Entity Hierarchy::add_entity(const std::string& name) {
        const engine::Entity entity = scene_->create_entity(name);
        selected_entity_ = entity;

        CLIENT_INFO("Created entity - {0}", name.c_str());
        return entity;
    }

    #ifdef ENABLE_IMGUI
    void Hierarchy::render_imgui() {
        engine::Entity entity_to_destroy{};

        ImGui::Begin("Hierarchy");
        if (ImGui::Button("+")) {
            ImGui::OpenPopup("AddEntityMenu");
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputText("##NameInput", name_buffer_, sizeof(name_buffer_));

        if (ImGui::BeginPopup("AddEntityMenu")) {
            auto menuItemWithIcon = [&](const char* label, EntityType type) -> bool {
                ImVec2 cursor = ImGui::GetCursorScreenPos();
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 22.0f);
                bool clicked = ImGui::MenuItem(label);
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                draw_entity_icon(draw_list, ImVec2(cursor.x + 4.0f, cursor.y + (ImGui::GetTextLineHeight() - 14.0f) * 0.5f), type);
                return clicked;
            };

            if (menuItemWithIcon("Entity", EntityType::Transform)) {
                const std::string name = name_buffer_[0] != '\0' ? name_buffer_ : "Entity";
                scene_->create_entity(name);
            }
            ImGui::Separator();

            auto create_primitive_entity = [&](const char* default_name, const char* primitive_path, EntityType type) {
                if (menuItemWithIcon(default_name, type)) {
                    const std::string name = name_buffer_[0] != '\0' ? name_buffer_ : default_name;
                    auto entity = scene_->create_entity(name);

                    auto handle = asset_manager_->get_handle_from_path(primitive_path);
                    entity.add_component<engine::MeshComponent>(handle);
                    entity.add_component<engine::MaterialComponent>(glm::vec4{1.0f, 0.5f, 0.2f, 1.0f});
                    entity.add_component<engine::BoxColliderComponent>();
                }
            };

            create_primitive_entity("Plane", "primitive://plane", EntityType::Mesh2D);
            create_primitive_entity("Circle", "primitive://circle", EntityType::Mesh2D);
            ImGui::Separator();
            create_primitive_entity("Cube", "primitive://cube", EntityType::Mesh3D);
            create_primitive_entity("Sphere", "primitive://sphere", EntityType::Mesh3D);
            create_primitive_entity("Cylinder", "primitive://cylinder", EntityType::Mesh3D);

            ImGui::Separator();
            if (menuItemWithIcon("Camera", EntityType::Camera)) {
                const std::string name = name_buffer_[0] != '\0' ? name_buffer_ : "Camera";
                auto entity = scene_->create_entity(name);
                entity.add_component<engine::CameraComponent>();
            }
            ImGui::Separator();
            if (menuItemWithIcon("Directional Light", EntityType::Light)) {
                const std::string name = name_buffer_[0] != '\0' ? name_buffer_ : "Sun";
                auto entity = scene_->create_entity(name);
                auto& transform = entity.get_component<engine::TransformComponent>();
                transform.rotation = glm::vec3(-90.0f, 135.0f, -45.0f);
                transform.position = glm::vec3(10.0f, 10.0f, 0.0f);
                entity.add_component<engine::DirectionalLightComponent>(
                    glm::vec3(1.0f, 0.95f, 0.8f),
                    1.2f
                );
            }
            if (menuItemWithIcon("Point Light", EntityType::Light)) {
                const std::string name = name_buffer_[0] != '\0' ? name_buffer_ : "Light";
                auto entity = scene_->create_entity(name);
                auto& transform = entity.get_component<engine::TransformComponent>();
                transform.rotation = glm::vec3(0.0f);
                transform.position = glm::vec3(0.0f);
                entity.add_component<engine::PointLightComponent>(
                    glm::vec3(1.0f, 0.95f, 0.8f),
                    1.2f
                );
            }
            ImGui::Separator();
            if (ImGui::BeginMenu("Imported Models")) {
                bool found_any = false;
                for (const auto& [handle, meta] : asset_manager_->get_registry()) {
                    if (meta.type == engine::AssetType::MESH && !meta.filepath.string().starts_with("primitive://")) {
                        found_any = true;
                        std::string filename = meta.filepath.filename().string();

                        if (menuItemWithIcon(filename.c_str(), EntityType::Mesh3D)) {
                            const std::string name = name_buffer_[0] != '\0' ? name_buffer_ : meta.filepath.stem().string();
                            auto entity = scene_->create_entity(name);
                            entity.add_component<engine::MeshComponent>(handle);
                            entity.add_component<engine::MaterialComponent>(glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});
                        }
                    }
                }
                if (!found_any) {
                    ImGui::TextDisabled("No models found");
                }
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }

        ImGui::Separator();
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 1.0f));

        auto view = scene_->view<engine::LabelComponent>();
        for (auto raw_entity : view) {
            engine::Entity entity{raw_entity, scene_.get()};
            const auto& label = entity.get_component<engine::LabelComponent>();
            ImGui::PushID(entity.uuid());

            bool is_active = true;
            if (entity.has_component<engine::ActiveComponent>()) {
                is_active = entity.get_component<engine::ActiveComponent>().active;
            }

            // ---------------------------------
            // Procedural Eye Visibility Button
            // ---------------------------------
            const ImVec2 eye_button_size(20.0f, ImGui::GetFrameHeight());
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.28f, 0.40f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.20f, 0.22f, 0.60f));
            if (ImGui::Button("##eye", eye_button_size)) {
                if (!entity.has_component<engine::ActiveComponent>()) {
                    entity.add_component<engine::ActiveComponent>(!is_active);
                }
                else {
                    auto& [active] = entity.get_component<engine::ActiveComponent>();
                    active = !active;
                }
            }
            const bool eye_hovered = ImGui::IsItemHovered();
            const ImVec2 eye_p0 = ImGui::GetItemRectMin();
            const float icon_x = eye_p0.x + (eye_button_size.x - 14.0f) * 0.5f;
            const float icon_y = eye_p0.y + (eye_button_size.y - 14.0f) * 0.5f;
            draw_eye_icon(ImGui::GetWindowDrawList(), ImVec2(icon_x, icon_y), is_active, eye_hovered);
            ImGui::PopStyleColor(3);
            // ---------------------------------

            ImGui::SameLine(0.0f, 4.0f);
            if (!is_active) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
            }
            const bool is_selected = selected_entity_ == raw_entity;
            const ImGuiTreeNodeFlags flags = (is_selected ? ImGuiTreeNodeFlags_Selected : 0)
                | ImGuiTreeNodeFlags_OpenOnArrow
                | ImGuiTreeNodeFlags_SpanAvailWidth;

            const bool opened = ImGui::TreeNodeEx(label.tag.c_str(), flags);

            if (!is_active) {
                ImGui::PopStyleColor();
            }
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

        ImGui::PopStyleVar();

        if (selected_entity_ && !ImGui::IsAnyItemActive() && engine::Input::is_key_pressed_once(engine::Key::DELETE)) {
            entity_to_destroy = selected_entity_;
        }
        if (entity_to_destroy) {
            if (selected_entity_ == entity_to_destroy) {
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
    #endif
}
