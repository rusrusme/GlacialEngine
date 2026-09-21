//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "EditorUILayer.hpp"

#include "EditorLayer.hpp"

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <commdlg.h>
#endif

#ifdef _WIN32
namespace {
    std::string open_file_dialog(const char* filter = "Scene Files (*.scene)\0*.scene\0All Files (*.*)\0*.*\0") {
        OPENFILENAMEA ofn;
        char sz_file[260] = {0};

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL; // Pass active window HWND if available
        ofn.lpstrFile = sz_file;
        ofn.nMaxFile = sizeof(sz_file);
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        // OFN_NOCHANGEDIR is critical to prevent Windows from altering
        // the application's working directory when selecting files
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileNameA(&ofn) == TRUE) {
            return std::string(ofn.lpstrFile);
        }
        return "";
    }
}
#endif

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
            // If gizmo is currently active, cancel the operation and restore initial transform
            if (was_using_gizmo_ || ImGuizmo::IsUsing()) {
                gizmo_canceled_ = true;
                ImGui::ClearActiveID();

                auto selected_entity = hierarchy_.get_selected_entity();
                if (selected_entity && selected_entity.has_component<engine::TransformComponent>()) {
                    selected_entity.get_component<engine::TransformComponent>() = initial_transform_;
                }
            }
            // Otherwise, deselect the object
            else {
                hierarchy_.set_selected_entity({});
            }
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

        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), dockspace_flags);

        ImGuiDockNode* node = ImGui::DockBuilderGetNode(dockspace_id);
        if (node == nullptr || node->ChildNodes[0] == nullptr) {
            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);

            ImGuiID dock_main_id = dockspace_id;
            ImGuiID dock_left_bottom_id;
            ImGuiID dock_bottom_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.25f, nullptr, &dock_main_id);
            ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.15f, nullptr, &dock_main_id);
            ImGuiID dock_left_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, nullptr, &dock_main_id);
            ImGuiID dock_left_top_id = ImGui::DockBuilderSplitNode(dock_left_id, ImGuiDir_Up, 0.4f, nullptr, &dock_left_bottom_id);

            ImGui::DockBuilderDockWindow("Asset Browser", dock_left_top_id);
            ImGui::DockBuilderDockWindow("Hierarchy", dock_left_bottom_id);
            ImGui::DockBuilderDockWindow("Inspector", dock_right_id);
            ImGui::DockBuilderDockWindow("Console", dock_bottom_id);
            ImGui::DockBuilderDockWindow("Scene", dock_main_id);

            ImGuiID float_node_id = ImGui::DockBuilderAddNode();
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::DockBuilderSetNodePos(float_node_id, ImVec2(viewport->Pos.x + 100.0f, viewport->Pos.y + 100.0f));
            ImGui::DockBuilderSetNodeSize(float_node_id, ImVec2(400.0f, 500.0f));

            ImGui::DockBuilderDockWindow("Debug Panel", float_node_id);
            ImGui::DockBuilderDockWindow("Shader Library Inspector", float_node_id);
            ImGui::DockBuilderFinish(float_node_id);

            ImGui::DockBuilderFinish(dockspace_id);
        }

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
        float panel_w = viewport_panel_size.x;
        if (panel_w < 1.0f) panel_w = 1.0f;
        float panel_h = viewport_panel_size.y;
        if (panel_h < 1.0f) panel_h = 1.0f;

        viewport_bounds_min_ = ImGui::GetCursorScreenPos();
        viewport_bounds_size_ = {panel_w, panel_h};

        // 1. Framebuffer Resizing
        if (editor_layer_) {
            uint32_t current_fb_w = renderer.get_framebuffer_width();
            uint32_t current_fb_h = renderer.get_framebuffer_height();

            if (current_fb_w != static_cast<uint32_t>(panel_w) ||
                current_fb_h != static_cast<uint32_t>(panel_h)) {
                renderer.on_resize(panel_w, panel_h);
            }
        }

        // Szene-Textur zeichnen
        uint32_t texture_id = renderer.get_final_color_texture();
        if (texture_id != 0) {
            ImGui::Image(texture_id, viewport_panel_size, ImVec2(0, 1), ImVec2(1, 0));
        }

        const bool viewport_clicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);

        // Drag & Drop Target
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

        // ImGuizmo zeichnen (aktualisiert ImGuizmo::IsOver() für diesen Frame)
        ImGuizmo::BeginFrame();
        if (camera_) {
            draw_imguizmo(hierarchy_.get_selected_entity(), *camera_, viewport_bounds_min_, viewport_panel_size);
        }

        // Picking/Selection ausführen (NUR wenn das Bild geklickt wurde UND nicht auf den Gizmo geklickt wurde)
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

        auto& transform = selected_entity.get_component<engine::TransformComponent>();

        if (gizmo_canceled_) {
            transform = initial_transform_;
            ImGuizmo::Enable(false);

            if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                gizmo_canceled_ = false;
                was_using_gizmo_ = false;
            }
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
        glm::mat4 transform_matrix = transform.get_matrix();

        const bool snap = engine::Input::is_key_pressed(engine::Key::LEFT_CONTROL) || engine::Input::is_key_pressed(engine::Key::RIGHT_CONTROL);
        const float snap_value = current_gizmo_operation_ == ImGuizmo::ROTATE ? 45.0f : 0.5f;
        const float snap_values[3] = {snap_value, snap_value, snap_value};

        ImGuizmo::Manipulate(
            glm::value_ptr(view_matrix),
            glm::value_ptr(proj_matrix),
            current_gizmo_operation_,
            current_gizmo_mode_,
            glm::value_ptr(transform_matrix),
            nullptr,
            snap ? snap_values : nullptr
        );

        const bool is_using = ImGuizmo::IsUsing();

        if (is_using && !was_using_gizmo_) {
            was_using_gizmo_ = true;
            gizmo_canceled_ = false;
            initial_transform_ = transform;
        }

        if (!is_using && was_using_gizmo_) {
            was_using_gizmo_ = false;
        }

        if (is_using) {
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

    void EditorUILayer::draw_imgui_menu_bar() {
        static bool show_save_popup = false;
        static char path_buffer[256] = "assets/scenes/default.scene";

        const bool is_ctrl = engine::Input::is_key_pressed(engine::Key::LEFT_CONTROL) ||
            engine::Input::is_key_pressed(engine::Key::RIGHT_CONTROL);

        auto handle_load_scene = [this] {
            #ifdef _WIN32
            std::string selected_file = open_file_dialog();
            if (!selected_file.empty()) {
                engine::SceneSerializer serializer(active_scene_);
                serializer.deserialize(selected_file);
            }
            #endif
        };

        if (is_ctrl && !engine::Input::is_keyboard_captured() && !ImGuizmo::IsUsing()) {
            if (engine::Input::is_key_pressed_once(engine::Key::N)) {
                engine::SceneSerializer serializer(active_scene_);
                serializer.deserialize("assets/scenes/clean.scene");
            }
            if (engine::Input::is_key_pressed_once(engine::Key::S)) {
                show_save_popup = true;
            }
            if (engine::Input::is_key_pressed_once(engine::Key::O)) {
                handle_load_scene();
            }
        }

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                    engine::SceneSerializer serializer(active_scene_);
                    serializer.deserialize("assets/scenes/clean.scene");
                }
                if (ImGui::MenuItem("Save Scene As", "Ctrl+S")) {
                    show_save_popup = true;
                }
                if (ImGui::MenuItem("Load Scene", "Ctrl+O")) {
                    handle_load_scene();
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
    }
    #endif
}
