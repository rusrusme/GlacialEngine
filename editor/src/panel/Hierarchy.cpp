//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Hierarchy.hpp"

#include "misc/Icons.hpp"

namespace editor {
    #ifdef ENABLE_IMGUI
    void Hierarchy::render_imgui(const engine::SystemContext& context) {
        const auto scene = context.scenes.active_scene();
        if (!scene) return;
        engine::Entity entity_to_destroy{};
        engine::Entity entity_to_duplicate{};

        ImGui::Begin("Hierarchy");

        render_header_bar();

        if (ImGui::BeginPopup("AddEntityMenu")) {
            render_add_entity_popup(context);
            ImGui::EndPopup();
        }
        ImGui::Separator();

        render_entity_list(entity_to_destroy, entity_to_duplicate, scene);

        process_shortcuts(entity_to_destroy, entity_to_duplicate);
        process_deferred_actions(entity_to_destroy, entity_to_duplicate, scene);
        process_deselection_inputs();

        ImGui::End();
    }
    #endif

    void Hierarchy::update_context(const std::shared_ptr<engine::Scene>& active_scene) {
        if (!active_scene) return;

        selected_entity_ = {};
        if (selected_uuid_ != 0) {
            bool found = false;
            auto view = active_scene->view<engine::IDComponent>();
            for (auto entity : view) {
                if (view.get<engine::IDComponent>(entity).id == selected_uuid_) {
                    selected_entity_ = {entity, active_scene.get()};
                    found = true;
                    break;
                }
            }
            if (!found) { selected_uuid_ = 0; }
        }
    }

    void Hierarchy::set_selected_entity(const engine::Entity new_entity) {
        selected_entity_ = new_entity;
        if (selected_entity_ && selected_entity_.has_component<engine::IDComponent>()) {
            selected_uuid_ = selected_entity_.get_component<engine::IDComponent>().id;
        }
        else {
            selected_uuid_ = 0;
        }
    }

    #ifdef ENABLE_IMGUI
    void Hierarchy::render_header_bar() {
        if (ImGui::Button("+")) {
            ImGui::OpenPopup("AddEntityMenu");
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputText("##NameInput", name_buffer_, sizeof(name_buffer_));
    }

    void Hierarchy::render_add_entity_popup(const engine::SystemContext& context) {
        const auto scene = context.scenes.active_scene();
        if (!scene) return;
        if (menu_item_with_icon("Entity", EntityType::Transform)) {
            scene->create_entity(resolve_entity_name("Entity"));
        }
        ImGui::Separator();
        render_primitives_submenus(context);

        ImGui::Separator();
        if (menu_item_with_icon("Camera", EntityType::Camera)) {
            auto entity = scene->create_entity(resolve_entity_name("Camera"));
            entity.add_component<engine::CameraComponent>();
        }
        ImGui::Separator();
        render_lighting_submenu(scene);

        ImGui::Separator();
        render_imported_models_submenu(context);
    }

    void Hierarchy::render_entity_list(engine::Entity& out_destroy, engine::Entity& out_duplicate, const std::shared_ptr<engine::Scene>& active_scene) {
        if (!active_scene) return;

        auto view = active_scene->view<engine::LabelComponent>();

        std::vector<entt::entity> entities;
        entities.reserve(view.size());
        for (auto raw_entity : view) {
            entities.push_back(raw_entity);
        }

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 1.0f));

        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(entities.size()));

        while (clipper.Step()) {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                engine::Entity entity{entities[i], active_scene.get()};
                render_entity_node(entity, out_destroy, out_duplicate);
            }
        }

        ImGui::PopStyleVar();
    }

    void Hierarchy::render_primitives_submenus(const engine::SystemContext& context) {
        create_primitive_entity("Plane", "primitive://plane", EntityType::Mesh2D, context);
        create_primitive_entity("Circle", "primitive://circle", EntityType::Mesh2D, context);
        ImGui::Separator();
        create_primitive_entity("Cube", "primitive://cube", EntityType::Mesh3D, context);
        create_primitive_entity("Sphere", "primitive://sphere", EntityType::Mesh3D, context);
        create_primitive_entity("Cylinder", "primitive://cylinder", EntityType::Mesh3D, context);
        create_primitive_entity("Pyramid", "primitive://pyramid", EntityType::Mesh3D, context);
    }

    void Hierarchy::render_lighting_submenu(const std::shared_ptr<engine::Scene>& active_scene) {
        if (!active_scene) return;
        if (menu_item_with_icon("Directional Light", EntityType::Light)) {
            auto entity = active_scene->create_entity(resolve_entity_name("Sun"));
            auto& transform = entity.get_component<engine::TransformComponent>();
            transform.rotation = glm::vec3(-90.0f, 135.0f, -45.0f);
            transform.position = glm::vec3(10.0f, 10.0f, 0.0f);
            entity.add_component<engine::DirectionalLightComponent>(glm::vec3(1.0f, 0.95f, 0.8f), 1.2f);
        }

        if (menu_item_with_icon("Point Light", EntityType::Light)) {
            auto entity = active_scene->create_entity(resolve_entity_name("Light"));
            auto& transform = entity.get_component<engine::TransformComponent>();
            transform.rotation = glm::vec3(0.0f);
            transform.position = glm::vec3(0.0f);
            entity.add_component<engine::PointLightComponent>(glm::vec3(1.0f, 0.95f, 0.8f), 1.2f);
        }
    }

    void Hierarchy::render_imported_models_submenu(const engine::SystemContext& context) {
        const auto scene = context.scenes.active_scene();
        if (!scene) return;
        if (!ImGui::BeginMenu("Imported Models")) return;

        bool found_any = false;
        for (const auto& [handle, meta] : context.assets.get_registry()) {
            const bool is_mesh = (meta.type == engine::AssetType::MESH);
            const bool is_user_model = !meta.filepath.string().starts_with("primitive://");

            if (is_mesh && is_user_model) {
                found_any = true;
                std::string filename = meta.filepath.filename().string();

                if (menu_item_with_icon(filename.c_str(), EntityType::Mesh3D)) {
                    auto entity = scene->create_entity(resolve_entity_name(meta.filepath.stem().string().c_str()));
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

    bool Hierarchy::menu_item_with_icon(const char* label, EntityType type) {
        ImVec2 cursor = ImGui::GetCursorScreenPos();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 22.0f);

        bool clicked = ImGui::MenuItem(label);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 icon_pos(cursor.x + 4.0f, cursor.y + (ImGui::GetTextLineHeight() - 14.0f) * 0.5f);
        draw_entity_icon(draw_list, icon_pos, type);

        return clicked;
    }

    void Hierarchy::create_primitive_entity(const char* label, const char* path, EntityType type, const engine::SystemContext& context) {
        const auto scene = context.scenes.active_scene();
        if (!scene) return;
        if (menu_item_with_icon(label, type)) {
            auto entity = scene->create_entity(resolve_entity_name(label));
            auto handle = context.assets.get_handle_from_path(path);

            entity.add_component<engine::MeshComponent>(handle);
            entity.add_component<engine::MaterialComponent>(glm::vec4{1.0f, 0.5f, 0.2f, 1.0f});
            entity.add_component<engine::BoxColliderComponent>();
        }
    }

    std::string Hierarchy::resolve_entity_name(const char* default_name) const {
        return name_buffer_[0] != '\0' ? name_buffer_ : default_name;
    }

    void Hierarchy::render_entity_node(engine::Entity entity, engine::Entity& out_destroy, engine::Entity& out_duplicate) {
        const auto& label = entity.get_component<engine::LabelComponent>();
        ImGui::PushID(entity.uuid());

        bool is_active = entity.has_component<engine::ActiveComponent>()
                             ? entity.get_component<engine::ActiveComponent>().active
                             : true;

        // Visibility Button
        if (render_eye_toggle_button(is_active)) {
            if (!entity.has_component<engine::ActiveComponent>()) {
                entity.add_component<engine::ActiveComponent>(!is_active);
            }
            else {
                entity.get_component<engine::ActiveComponent>().active = !is_active;
            }
        }

        ImGui::SameLine(0.0f, 4.0f);
        if (!is_active) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        }

        const bool is_selected = (selected_entity_ == entity);
        const ImGuiTreeNodeFlags flags = (is_selected ? ImGuiTreeNodeFlags_Selected : 0)
            | ImGuiTreeNodeFlags_OpenOnArrow
            | ImGuiTreeNodeFlags_SpanAvailWidth;

        const bool opened = ImGui::TreeNodeEx(label.tag.c_str(), flags);

        if (!is_active) {
            ImGui::PopStyleColor();
        }

        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            set_selected_entity(entity);
        }

        render_entity_context_menu(entity, out_destroy, out_duplicate);

        if (opened) {
            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    bool Hierarchy::render_eye_toggle_button(bool is_active) {
        const ImVec2 eye_button_size(20.0f, ImGui::GetFrameHeight());

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.28f, 0.40f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.20f, 0.22f, 0.60f));

        bool toggled = ImGui::Button("##eye", eye_button_size);

        const bool eye_hovered = ImGui::IsItemHovered();
        const ImVec2 eye_p0 = ImGui::GetItemRectMin();
        const float icon_x = eye_p0.x + (eye_button_size.x - 14.0f) * 0.5f;
        const float icon_y = eye_p0.y + (eye_button_size.y - 14.0f) * 0.5f;

        draw_eye_icon(ImGui::GetWindowDrawList(), ImVec2(icon_x, icon_y), is_active, eye_hovered);

        ImGui::PopStyleColor(3);
        return toggled;
    }

    void Hierarchy::render_entity_context_menu(const engine::Entity entity, engine::Entity& out_destroy, engine::Entity& out_duplicate) {
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Delete")) out_destroy = entity;
            if (ImGui::MenuItem("Duplicate")) out_duplicate = entity;
            ImGui::EndPopup();
        }
    }

    void Hierarchy::process_shortcuts(engine::Entity& out_destroy, engine::Entity& out_duplicate) {
        if (!selected_entity_ || ImGui::IsAnyItemActive()) return;

        if (engine::Input::is_key_pressed_once(engine::Key::DELETE)) {
            out_destroy = selected_entity_;
        }
        if (engine::Input::is_key_pressed(engine::Key::LEFT_CONTROL) && engine::Input::is_key_pressed_once(engine::Key::D)) {
            out_duplicate = selected_entity_;
        }
    }

    void Hierarchy::process_deferred_actions(engine::Entity entity_to_destroy, engine::Entity entity_to_duplicate, const std::shared_ptr<engine::Scene>& active_scene) {
        if (!active_scene) return;
        if (entity_to_duplicate) {
            set_selected_entity(active_scene->duplicate_entity(entity_to_duplicate));
        }
        if (entity_to_destroy) {
            if (selected_entity_ == entity_to_destroy) {
                set_selected_entity({});
            }
            active_scene->destroy_entity(entity_to_destroy);
        }
    }

    void Hierarchy::process_deselection_inputs() {
        const bool clicked_empty_space = ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered();
        const bool pressed_escape = ImGui::IsWindowFocused() && engine::Input::is_key_pressed_once(engine::Key::ESCAPE);

        if (clicked_empty_space || pressed_escape) {
            set_selected_entity({});
        }
    }
    #endif
}
