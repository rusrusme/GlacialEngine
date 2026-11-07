//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Hierarchy.hpp"

#include "Config.hpp"
#include "asset/AssetManager.hpp"
#include "command/CommandHistory.hpp"
#include "command/EditorCommands.hpp"
#include "command/Transaction.hpp"
#include "core/SystemContext.hpp"
#include "event/Shortcuts.hpp"
#include "project/scene/SceneManager.hpp"

namespace editor {
    void Hierarchy::on_update(SystemContext& context, float delta_time) {
        auto active_scene = context.scenes.active_scene();
        if (!active_scene) return;

        selected_entity_ = {};
        if (selected_uuid_ != 0) {
            bool found = false;
            for (auto view = active_scene->view<IDComponent>(); auto entity : view) {
                if (view.get<IDComponent>(entity).id == selected_uuid_) {
                    selected_entity_ = {entity, active_scene.get()};
                    found = true;
                    break;
                }
            }
            if (!found) { selected_uuid_ = 0; }
        }
    }

    void Hierarchy::on_render_ui(SystemContext& context) {
        if (!visible_) return;

        const auto scene = context.scenes.active_scene();
        if (!scene) return;
        ImGui::Begin("Hierarchy");

        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
            Shortcuts::set_active_context(shortcut_context());
        }

        render_header_bar();

        if (ImGui::BeginPopup("AddEntityMenu")) {
            render_add_entity_popup(context);
            ImGui::EndPopup();
        }
        ImGui::Separator();

        render_entity_list(scene);

        process_deferred_actions(context);
        process_deselection_inputs();

        ImGui::End();
    }

    void Hierarchy::load_config() {
        visible_ = config::get_value("Hierarchy_visible", visible_);
    }

    void Hierarchy::save_config() {
        config::set_value("Hierarchy_visible", visible_);
    }

    void Hierarchy::set_selected_entity(Entity new_entity) {
        selected_entity_ = new_entity;
        if (selected_entity_ && selected_entity_.has_component<IDComponent>()) {
            selected_uuid_ = selected_entity_.get_component<IDComponent>().id;
        }
        else {
            selected_uuid_ = 0;
        }
    }

    void Hierarchy::delete_entity() {
        if (selected_entity_) {
            entity_to_destroy_ = selected_entity_;
        }
    }

    void Hierarchy::duplicate_entity() {
        if (selected_entity_) {
            entity_to_duplicate_ = selected_entity_;
        }
    }

    void Hierarchy::rename_entity() {
        if (selected_entity_) {
            start_renaming(selected_entity_);
        }
    }

    void Hierarchy::deselect_entity() {
        set_selected_entity({});
    }

    void Hierarchy::render_header_bar() {
        if (ImGui::Button("+")) {
            ImGui::OpenPopup("AddEntityMenu");
        }
    }

    void Hierarchy::render_add_entity_popup(const SystemContext& context) {
        const auto scene = context.scenes.active_scene();
        if (!scene) return;
        if (menu_item_with_icon("Entity", EntityType::Transform)) {
            scene->create_entity("Entity");
        }
        ImGui::Separator();
        render_primitives_submenus(context);

        ImGui::Separator();
        if (menu_item_with_icon("Camera", EntityType::Camera)) {
            auto entity = scene->create_entity("Camera");
            entity.add_component<CameraComponent>();
        }
        ImGui::Separator();
        render_lighting_submenu(scene);

        ImGui::Separator();
        render_imported_models_submenu(context);
    }

    void Hierarchy::render_entity_list(const std::shared_ptr<Scene>& active_scene) {
        if (!active_scene) return;

        auto view = active_scene->view<LabelComponent>();

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
                Entity entity{entities[i], active_scene.get()};
                render_entity_node(entity);
            }
        }

        ImGui::PopStyleVar();
    }

    void Hierarchy::render_primitives_submenus(const SystemContext& context) {
        create_primitive_entity("Plane", "primitive://plane", EntityType::Mesh2D, context);
        create_primitive_entity("Circle", "primitive://circle", EntityType::Mesh2D, context);
        ImGui::Separator();
        create_primitive_entity("Cube", "primitive://cube", EntityType::Mesh3D, context);
        create_primitive_entity("Sphere", "primitive://sphere", EntityType::Mesh3D, context);
        create_primitive_entity("Cylinder", "primitive://cylinder", EntityType::Mesh3D, context);
        create_primitive_entity("Pyramid", "primitive://pyramid", EntityType::Mesh3D, context);
    }

    void Hierarchy::render_lighting_submenu(const std::shared_ptr<Scene>& active_scene) {
        if (!active_scene) return;
        if (menu_item_with_icon("Directional Light", EntityType::Light)) {
            auto entity = active_scene->create_entity("Sun");
            auto& transform = entity.get_component<TransformComponent>();
            transform.rotation = glm::vec3(-90.0f, 135.0f, -45.0f);
            transform.position = glm::vec3(10.0f, 10.0f, 0.0f);
            entity.add_component<DirectionalLightComponent>(glm::vec3(1.0f, 0.95f, 0.8f), 1.2f);
        }

        if (menu_item_with_icon("Point Light", EntityType::Light)) {
            auto entity = active_scene->create_entity("Light");
            auto& transform = entity.get_component<TransformComponent>();
            transform.rotation = glm::vec3(0.0f);
            transform.position = glm::vec3(0.0f);
            entity.add_component<PointLightComponent>(glm::vec3(1.0f, 0.95f, 0.8f), 1.2f);
        }
    }

    void Hierarchy::render_imported_models_submenu(const SystemContext& context) {
        const auto scene = context.scenes.active_scene();
        if (!scene) return;
        if (!ImGui::BeginMenu("Imported Models")) return;

        bool found_any = false;
        for (const auto& [handle, meta] : context.assets.registry()) {
            const bool is_mesh = (meta.type == AssetType::MESH);
            const bool is_user_model = !meta.filepath.string().starts_with("primitive://");

            if (is_mesh && is_user_model) {
                found_any = true;
                std::string filename = meta.filepath.filename().string();

                if (menu_item_with_icon(filename.c_str(), EntityType::Mesh3D)) {
                    auto entity = scene->create_entity(meta.filepath.stem().string());
                    entity.add_component<MeshComponent>(handle);
                    entity.add_component<MaterialComponent>(glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});
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

    void Hierarchy::create_primitive_entity(const char* label, const char* path, EntityType type, const SystemContext& context) {
        const auto scene = context.scenes.active_scene();
        if (!scene) return;
        if (menu_item_with_icon(label, type)) {
            auto entity = scene->create_entity(label);
            auto handle = context.assets.asset_handle_from_path(path);

            entity.add_component<MeshComponent>(handle);
            entity.add_component<MaterialComponent>(glm::vec4{1.0f, 0.5f, 0.2f, 1.0f});
            entity.add_component<BoxColliderComponent>();
        }
    }

    void Hierarchy::render_entity_node(Entity entity) {
        auto& label = entity.get_component<LabelComponent>();
        ImGui::PushID(entity.uuid());

        const bool is_active = entity.is_active();
        if (render_eye_toggle_button(is_active)) {
            entity.set_active(!is_active);
        }

        ImGui::SameLine(0.0f, 4.0f);
        if (!is_active) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        }

        if (renaming_entity_ == entity) {
            if (renaming_) {
                ImGui::SetKeyboardFocusHere();
                renaming_ = false;
            }

            ImGui::SetNextItemWidth(-1.0f);
            const bool enter_pressed = ImGui::InputText(
                "##RenameInput", rename_buffer_, sizeof(rename_buffer_),
                ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll
            );

            if (enter_pressed || (ImGui::IsItemDeactivated() && !ImGui::IsKeyPressed(ImGuiKey_Escape))) {
                if (rename_buffer_[0] != '\0' && label.tag != rename_buffer_) {
                    entity_to_rename_ = renaming_entity_;
                    new_entity_name_ = rename_buffer_;
                }
                renaming_entity_ = {};
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                renaming_entity_ = {};
            }
        }
        else {
            const bool is_selected = (selected_entity_ == entity);
            const ImGuiTreeNodeFlags flags = (is_selected ? ImGuiTreeNodeFlags_Selected : 0)
                | ImGuiTreeNodeFlags_OpenOnArrow
                | ImGuiTreeNodeFlags_SpanAvailWidth;

            const bool opened = ImGui::TreeNodeEx(label.tag.c_str(), flags);

            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
                set_selected_entity(entity);
            }

            render_entity_context_menu(entity);

            if (opened) {
                ImGui::TreePop();
            }
        }

        if (!is_active) {
            ImGui::PopStyleColor();
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

    void Hierarchy::render_entity_context_menu(Entity entity) {
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Rename", Shortcuts::text(Action::RenameEntity).data())) {
                start_renaming(entity);
            }
            if (ImGui::MenuItem("Duplicate", Shortcuts::text(Action::DuplicateEntity).data())) {
                entity_to_duplicate_ = entity;
            }
            if (ImGui::MenuItem("Delete", Shortcuts::text(Action::DeleteEntity).data())) {
                entity_to_destroy_ = entity;
            }
            ImGui::EndPopup();
        }
    }

    void Hierarchy::process_deferred_actions(SystemContext& context) {
        auto active_scene = context.scenes.active_scene();
        if (!active_scene) return;

        auto& history = context.get<CommandHistory>();

        // Delete Entity
        if (entity_to_destroy_) {
            if (selected_entity_ == entity_to_destroy_) {
                set_selected_entity({});
            }
            if (renaming_entity_ == entity_to_destroy_) {
                renaming_entity_ = {};
            }

            Transaction tx("Delete Entity", history);
            history.record(std::make_unique<DeleteEntityCommand>(entity_to_destroy_, context), context);

            entity_to_destroy_ = {};
        }

        // Duplicate Entity
        if (entity_to_duplicate_) {
            auto cmd = std::make_unique<DuplicateEntityCommand>(entity_to_duplicate_, context);
            const UUID new_uuid = cmd->duplicate_uuid();

            Transaction tx("Duplicate Entity", history);
            history.record(std::move(cmd), context);

            // Select the newly spawned duplicate
            selected_uuid_ = new_uuid;
            entity_to_duplicate_ = {};
        }

        // Rename Entity
        if (entity_to_rename_) {
            const auto& label = entity_to_rename_.get_component<LabelComponent>();
            if (label.tag != new_entity_name_ && !new_entity_name_.empty()) {
                Transaction tx("Rename Entity", history);
                history.record(std::make_unique<RenameEntityCommand>(
                                   entity_to_rename_.uuid(), label.tag, new_entity_name_
                               ), context);
            }
            entity_to_rename_ = {};
            new_entity_name_.clear();
        }
    }

    void Hierarchy::process_deselection_inputs() {
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered()) {
            set_selected_entity({});
        }
    }

    void Hierarchy::start_renaming(Entity entity) {
        if (!entity || !entity.has_component<LabelComponent>()) return;

        renaming_entity_ = entity;
        const auto& label = entity.get_component<LabelComponent>();
        std::strncpy(rename_buffer_, label.tag.c_str(), sizeof(rename_buffer_) - 1);
        rename_buffer_[sizeof(rename_buffer_) - 1] = '\0';
        renaming_ = true;
    }
}
