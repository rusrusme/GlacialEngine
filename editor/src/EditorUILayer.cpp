//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "EditorUILayer.hpp"

#include "EditorLayer.hpp"

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
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.FontGlobalScale = 1.0f;

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
        if (engine::Input::is_key_pressed_once(engine::Key::ESCAPE)) {
            hierarchy_.set_selected_entity({});
        }

        if (engine::Input::is_keyboard_captured() || ImGuizmo::IsUsing()) {
            return;
        }

        const bool is_ctrl = engine::Input::is_key_pressed(engine::Key::LEFT_CONTROL) || engine::Input::is_key_pressed(engine::Key::RIGHT_CONTROL);
        if (is_ctrl) {
            if (engine::Input::is_key_pressed_once(engine::Key::W)) current_gizmo_operation_ = ImGuizmo::TRANSLATE;
            if (engine::Input::is_key_pressed_once(engine::Key::E)) current_gizmo_operation_ = ImGuizmo::ROTATE;
            if (engine::Input::is_key_pressed_once(engine::Key::R)) current_gizmo_operation_ = ImGuizmo::SCALE;
        }
        #endif
    }

    void EditorUILayer::on_render(engine::Renderer& renderer, float delta_time) {
    }

    #ifdef ENABLE_IMGUI
    void EditorUILayer::on_imgui_render(engine::Renderer& renderer, float delta_time) {
        draw_imgui_menu_bar();

        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

        // ------------------------------------------------------------------
        // SCENE PANEL
        // ------------------------------------------------------------------
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Scene");

        is_viewport_hovered_ = ImGui::IsWindowHovered();
        is_viewport_focused_ = ImGui::IsWindowFocused();
        if (editor_layer_) {
            editor_layer_->set_viewport_hovered(is_viewport_hovered_);
            editor_layer_->set_viewport_focused(is_viewport_focused_);
        }

        ImVec2 viewport_panel_size = ImGui::GetContentRegionAvail();
        if (viewport_panel_size.x < 1.0f) viewport_panel_size.x = 1.0f;
        if (viewport_panel_size.y < 1.0f) viewport_panel_size.y = 1.0f;

        viewport_bounds_min_ = ImGui::GetCursorScreenPos();
        viewport_bounds_size_ = {viewport_panel_size.x, viewport_panel_size.y};

        // 1. Framebuffer Resizing
        if (editor_layer_) {
            uint32_t current_fb_w = editor_layer_->get_framebuffer_width();
            uint32_t current_fb_h = editor_layer_->get_framebuffer_height();

            if (current_fb_w != static_cast<uint32_t>(viewport_panel_size.x) ||
                current_fb_h != static_cast<uint32_t>(viewport_panel_size.y)) {
                editor_layer_->resize_viewport(
                    static_cast<uint32_t>(viewport_panel_size.x),
                    static_cast<uint32_t>(viewport_panel_size.y)
                );
            }
        }

        // 2. Szene-Textur zeichnen
        uint32_t texture_id = editor_layer_ ? editor_layer_->get_scene_texture_id() : 0;
        if (texture_id != 0) {
            ImGui::Image((void*)(intptr_t)texture_id, viewport_panel_size, ImVec2(0, 1), ImVec2(1, 0));
        }

        // WICHTIG: Prüfen, ob das 3D-Viewport-Bild mit Links geklickt wurde
        const bool viewport_clicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);

        // 3. Drag & Drop Target
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM")) {
                engine::AssetHandle dropped_handle = *static_cast<const engine::AssetHandle*>(payload->Data);
                const auto& meta = asset_manager_->get_metadata(dropped_handle);

                if (meta.is_valid() && meta.type == engine::AssetType::MESH) {
                    std::string entity_name = meta.filepath.stem().string();
                    auto entity = active_scene_->create_entity(entity_name);
                    entity.add_component<engine::MeshComponent>(dropped_handle);
                    entity.add_component<engine::MaterialComponent>(glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});

                    ImVec2 mouse_pos = ImGui::GetMousePos();
                    glm::vec2 local_mouse = {mouse_pos.x - viewport_bounds_min_.x, mouse_pos.y - viewport_bounds_min_.y};

                    if (camera_) {
                        const engine::Ray ray = camera_->screen_point_to_ray(local_mouse, viewport_bounds_size_);
                        glm::vec3 hit_point{0.0f};

                        if (ray.intersect_plane_y(0.0f, hit_point)) {
                            auto& transform = entity.get_component<engine::TransformComponent>();
                            transform.position = hit_point;
                        }
                    }

                    hierarchy_.set_selected_entity(entity);
                }
            }
            ImGui::EndDragDropTarget();
        }

        // 4. ImGuizmo zeichnen (aktualisiert ImGuizmo::IsOver() für diesen Frame)
        ImGuizmo::BeginFrame();
        if (camera_) {
            draw_imguizmo(hierarchy_.get_selected_entity(), *camera_, viewport_bounds_min_, viewport_panel_size);
        }

        // 5. Picking/Selection ausführen (NUR wenn das Bild geklickt wurde UND nicht auf den Gizmo geklickt wurde)
        if (viewport_clicked && !ImGuizmo::IsOver() && !ImGuizmo::IsUsing()) {
            ImVec2 mouse_pos = ImGui::GetMousePos();
            glm::vec2 local_mouse = {
                mouse_pos.x - viewport_bounds_min_.x,
                mouse_pos.y - viewport_bounds_min_.y
            };

            const engine::Ray ray = camera_->screen_point_to_ray(local_mouse, viewport_bounds_size_);

            if (auto hit = engine::CollisionSystem::raycast(*active_scene_, ray, 1000.0f, asset_manager_.get())) {
                hierarchy_.set_selected_entity(hit->entity);
            }
            else {
                hierarchy_.set_selected_entity({});
            }
        }

        ImGui::End();
        ImGui::PopStyleVar();

        // Übrige Panels
        hierarchy_.render_imgui();
        object_inspector_.set_selected_entity(hierarchy_.get_selected_entity());
        object_inspector_.render_imgui();
        asset_browser_.render_imgui();
        console_.render_imgui();
    }
    #endif

    void EditorUILayer::on_event(engine::Event& event) {
        engine::EventDispatcher dispatcher(event);
    }

    #ifdef ENABLE_IMGUI
    void EditorUILayer::draw_imguizmo(engine::Entity selected_entity, const engine::Camera& camera, const ImVec2& viewport_pos, const ImVec2& viewport_size) {
        if (!selected_entity || !selected_entity.has_component<engine::TransformComponent>()) {
            return;
        }

        ImGuizmo::Enable(true);
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
        ImGuizmo::SetRect(viewport_pos.x, viewport_pos.y, viewport_size.x, viewport_size.y);
        ImGuizmo::SetID(0);
        ImGuizmo::SetGizmoSizeClipSpace(0.115f);

        glm::mat4 view_matrix = camera.get_view_matrix();
        glm::mat4 proj_matrix = camera.get_projection_matrix();

        auto& transform = selected_entity.get_component<engine::TransformComponent>();
        glm::mat4 transform_matrix = transform.get_matrix();

        const bool snap = engine::Input::is_key_pressed(engine::Key::LEFT_CONTROL) || engine::Input::is_key_pressed(engine::Key::RIGHT_CONTROL);
        const float snap_value = current_gizmo_operation_ == ImGuizmo::ROTATE ? 45.0f : 0.5f;
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

        // Sobald der Gizmo bewegt/genutzt wird, schreiben wir die Werte direkt in die Component
        if (ImGuizmo::IsUsing()) {
            float translation[3], rotation[3], scale[3];
            ImGuizmo::DecomposeMatrixToComponents(
                glm::value_ptr(transform_matrix),
                translation,
                rotation,
                scale
            );

            transform.position = glm::make_vec3(translation);
            transform.rotation = glm::make_vec3(rotation);
            transform.scale = glm::make_vec3(scale);
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
