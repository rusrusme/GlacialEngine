//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "EditorUILayer.hpp"

namespace editor {
    #ifdef ENABLE_IMGUI
    namespace {
        ImGuizmo::OPERATION current_gizmo_operation_ = ImGuizmo::TRANSLATE;
        ImGuizmo::MODE current_gizmo_mode_ = ImGuizmo::LOCAL;
    }
    #endif

    EditorUILayer::EditorUILayer(std::shared_ptr<engine::Scene>& scene, std::shared_ptr<engine::AssetManager> asset_manager) : active_scene_(std::move(scene)), asset_manager_(std::move(asset_manager)) {}

    void EditorUILayer::on_attach(engine::Renderer& renderer) {
        object_inspector_.init(active_scene_, asset_manager_);
        hierarchy_.init(active_scene_, asset_manager_);
        asset_browser_.init(asset_manager_);

        #ifdef ENABLE_IMGUI
        ImGuizmo::Style& style = ImGuizmo::GetStyle();
        style.TranslationLineThickness = 3.0f;
        style.TranslationLineArrowSize = 5.0f;
        style.CenterCircleSize = 4.0f;
        style.HatchedAxisLineThickness = 0.0f;
        #endif
    }

    void EditorUILayer::on_detach() {
        object_inspector_.init(nullptr, nullptr);
        hierarchy_.init(nullptr, nullptr);
    }

    void EditorUILayer::on_update(float delta_time) {
        #ifdef ENABLE_IMGUI
        if (engine::Input::is_keyboard_captured() || ImGuizmo::IsUsing()) {
            return;
        }
        const bool is_ctrl = engine::Input::is_key_pressed(engine::Key::LEFT_CONTROL) || engine::Input::is_key_pressed(engine::Key::RIGHT_CONTROL);

        if (is_ctrl) {
            if (engine::Input::is_key_pressed_once(engine::Key::W)) {
                current_gizmo_operation_ = ImGuizmo::TRANSLATE;
            }
            if (engine::Input::is_key_pressed_once(engine::Key::E)) {
                current_gizmo_operation_ = ImGuizmo::ROTATE;
            }
            if (engine::Input::is_key_pressed_once(engine::Key::R)) {
                current_gizmo_operation_ = ImGuizmo::SCALE;
            }
        }
        #endif
    }

    void EditorUILayer::on_render(engine::Renderer& renderer, float delta_time) {
    }

    #ifdef ENABLE_IMGUI
    void EditorUILayer::on_imgui_render(engine::Renderer& renderer, float delta_time) {
        draw_imgui_menu_bar();
        ImGuizmo::BeginFrame();
        ImGuizmo::Enable(true);

        hierarchy_.render_imgui();
        object_inspector_.set_selected_entity(hierarchy_.get_selected_entity());
        object_inspector_.render_imgui();
        asset_browser_.render_imgui();

        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        ImVec2 viewport_pos = main_viewport->Pos;
        ImVec2 viewport_size = main_viewport->Size;

        if (camera_) {
            draw_imguizmo(hierarchy_.get_selected_entity(), *camera_, viewport_pos, viewport_size);
        }
    }
    #endif

    void EditorUILayer::on_event(engine::Event& event) {
        engine::EventDispatcher dispatcher(event);

        #ifdef ENABLE_IMGUI
        if (ImGuizmo::IsOver() || ImGuizmo::IsUsing()) {
            dispatcher.dispatch<engine::MouseButtonPressedEvent>([](engine::MouseButtonPressedEvent& _) {
                return true;
            });
            dispatcher.dispatch<engine::MouseMovedEvent>([](engine::MouseMovedEvent& _) {
                return true;
            });
            return;
        }
        #endif

        dispatcher.dispatch<engine::MouseButtonPressedEvent>([this](engine::MouseButtonPressedEvent& e) {
            if (e.button() != engine::Mouse::MOUSE_BUTTON_LEFT) {
                return false;
            }
            if (!camera_ || engine::Input::is_mouse_captured()) {
                return false;
            }
            const glm::vec2 mouse_pos = engine::Input::get_mouse_position();
            const uint32_t width = engine::Application::get().window().get_width();
            const uint32_t height = engine::Application::get().window().get_height();
            const glm::vec2 viewport_size = {static_cast<float>(width), static_cast<float>(height)};
            const engine::Ray ray = camera_->screen_point_to_ray(mouse_pos, viewport_size);

            if (auto hit = engine::CollisionSystem::raycast(*active_scene_, ray)) {
                hierarchy_.set_selected_entity(hit->entity);
            }
            else {
                hierarchy_.set_selected_entity({});
            }
            block_gizmo_until_mouse_up_ = true;
            return false;
        });
    }

    #ifdef ENABLE_IMGUI
    void EditorUILayer::draw_imguizmo(engine::Entity selected_entity, const engine::Camera& camera, const ImVec2& viewport_pos, const ImVec2& viewport_size) {
        if (!selected_entity || !selected_entity.has_component<engine::TransformComponent>()) {
            return;
        }

        if (block_gizmo_until_mouse_up_) {
            if (!engine::Input::is_mouse_button_pressed(engine::Mouse::MOUSE_BUTTON_LEFT)) {
                block_gizmo_until_mouse_up_ = false;
            }
            ImGuizmo::Enable(false);
        }
        else {
            ImGuizmo::Enable(true);
        }

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());
        ImGuizmo::SetRect(viewport_pos.x, viewport_pos.y, viewport_size.x, viewport_size.y);
        ImGuizmo::SetID(0);
        ImGuizmo::SetGizmoSizeClipSpace(0.115f);

        glm::mat4 view_matrix = camera.get_view_matrix();
        glm::mat4 proj_matrix = camera.get_projection_matrix();

        auto& transform = selected_entity.get_component<engine::TransformComponent>();

        if (ImGuizmo::IsUsing()) {
            if (!was_using_gizmo_) {
                initial_transform_ = transform;
                active_gizmo_matrix_ = transform.get_matrix();
                was_using_gizmo_ = true;
                gizmo_canceled_ = false;
            }
        }
        else {
            was_using_gizmo_ = false;
            gizmo_canceled_ = false;
        }

        glm::mat4 transform_matrix = was_using_gizmo_ ? active_gizmo_matrix_ : transform.get_matrix();

        bool snap = engine::Input::is_key_pressed(engine::Key::LEFT_CONTROL) || engine::Input::is_key_pressed(engine::Key::RIGHT_CONTROL);
        float snap_value = current_gizmo_operation_ == ImGuizmo::ROTATE ? 45.0f : 0.5f;
        float snap_values[3] = {snap_value, snap_value, snap_value};

        ImGuizmo::Manipulate(
            glm::value_ptr(view_matrix),
            glm::value_ptr(proj_matrix),
            current_gizmo_operation_,
            current_gizmo_mode_,
            glm::value_ptr(transform_matrix),
            nullptr,
            snap ? snap_values : nullptr
        );

        if (ImGuizmo::IsUsing()) {
            active_gizmo_matrix_ = transform_matrix;

            if (engine::Input::is_key_pressed(engine::Key::ESCAPE)) {
                gizmo_canceled_ = true;
            }

            if (gizmo_canceled_) {
                transform = initial_transform_;
            }
            else {
                float translation[3], rotation[3], scale[3];
                ImGuizmo::DecomposeMatrixToComponents(
                    glm::value_ptr(transform_matrix),
                    translation,
                    rotation,
                    scale
                );

                if (current_gizmo_operation_ == ImGuizmo::TRANSLATE) {
                    transform.position = glm::make_vec3(translation);
                }
                else if (current_gizmo_operation_ == ImGuizmo::ROTATE) {
                    transform.rotation = glm::make_vec3(rotation);
                }
                else if (current_gizmo_operation_ == ImGuizmo::SCALE) {
                    transform.scale = glm::make_vec3(scale);
                }
                else {
                    transform.position = glm::make_vec3(translation);
                    transform.rotation = glm::make_vec3(rotation);
                    transform.scale = glm::make_vec3(scale);
                }
            }
        }
    }

    void EditorUILayer::draw_imgui_menu_bar() const {
        static bool show_save_popup = false;
        static bool show_load_popup = false;
        static char path_buffer[256] = "assets/scenes/default.scene";

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Save Scene As...", "Ctrl+S")) {
                    show_save_popup = true;
                }
                if (ImGui::MenuItem("Load Scene...", "Ctrl+O")) {
                    show_load_popup = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // --- Save Scene Modal Dialog ---
        if (show_save_popup) {
            ImGui::OpenPopup("Save Scene");
            show_save_popup = false;
        }

        if (ImGui::BeginPopupModal("Save Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::InputText("File Path", path_buffer, sizeof(path_buffer));

            if (ImGui::Button("Save", ImVec2(120, 0))) {
                engine::SceneSerializer serializer(active_scene_);
                serializer.serialize(path_buffer);
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // --- Load Scene Modal Dialog ---
        if (show_load_popup) {
            ImGui::OpenPopup("Load Scene");
            show_load_popup = false;
        }

        if (ImGui::BeginPopupModal("Load Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::InputText("File Path", path_buffer, sizeof(path_buffer));

            if (ImGui::Button("Load", ImVec2(120, 0))) {
                engine::SceneSerializer serializer(active_scene_);
                if (!serializer.deserialize(path_buffer)) {
                    // optional: SPDLOG_ERROR("Failed to load scenes from {}", path_buffer);
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
    #endif
}
