//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "UILayer.hpp"

#include "EditorLayer.hpp"
#include "gui/Icons.hpp"
#include "gui/toolbar/Toolbar.hpp"

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
// #include <commdlg.h>
#endif

#ifdef _WIN32
namespace {
    std::string open_file_dialog(const char* filter = "Scene Files (*.scene)\0*.scene\0All Files (*.*)\0*.*\0") {
        OPENFILENAMEA ofn;
        char sz_file[260] = {0};

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = nullptr; // Pass active window HWND if available
        ofn.lpstrFile = sz_file;
        ofn.nMaxFile = sizeof(sz_file);
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        // OFN_NOCHANGEDIR is critical to prevent Windows from altering
        // the application's working directory when selecting files
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileNameA(&ofn) == TRUE) {
            return {ofn.lpstrFile};
        }
        return "";
    }

    std::string save_file_dialog(const char* filter = "Scene Files (*.scene)\0*.scene\0All Files (*.*)\0*.*\0") {
        OPENFILENAMEA ofn;
        char sz_file[260] = {0};

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = nullptr;
        ofn.lpstrFile = sz_file;
        ofn.nMaxFile = sizeof(sz_file);
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.lpstrDefExt = "scene"; // Automatically appends .scene if omitted by user
        // OFN_OVERWRITEPROMPT asks for confirmation before overwriting an existing file
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

        if (GetSaveFileNameA(&ofn) == TRUE) {
            // return std::string(ofn.lpstrFile);
            return {ofn.lpstrFile};
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

    UILayer::UILayer(std::shared_ptr<engine::Scene>& scene, std::shared_ptr<engine::AssetManager> asset_manager) : active_scene_(std::move(scene)), asset_manager_(std::move(asset_manager)) {}

    void UILayer::on_attach(engine::Renderer& renderer) {
        object_inspector_.init(active_scene_, asset_manager_, audio_layer_);
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

        register_default_toolbar_items();
    }

    void UILayer::on_detach() {
        object_inspector_.init(nullptr, nullptr, nullptr);
        hierarchy_.init(nullptr, nullptr);
    }

    void UILayer::on_update(float delta_time) {
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

    void UILayer::on_render(engine::Renderer& renderer, float delta_time) {
    }

    #ifdef ENABLE_IMGUI
    void UILayer::on_imgui_render(engine::Renderer& renderer, float delta_time) {
        if (editor_layer_) {
            set_scene_context(editor_layer_->get_active_scene());
        }

        draw_imgui_menu_bar();

        // Clean DockSpace across the viewport
        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), dockspace_flags);

        ImGuiDockNode* node = ImGui::DockBuilderGetNode(dockspace_id);
        if (node == nullptr || node->ChildNodes[0] == nullptr) {
            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);

            ImGuiID dock_main_id = dockspace_id;

            // Split a node at the top specifically for the Toolbar container
            ImGuiID dock_top_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Up, 0.06f, nullptr, &dock_main_id);

            ImGuiID dock_left_bottom_id;
            ImGuiID dock_bottom_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.25f, nullptr, &dock_main_id);
            ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.15f, nullptr, &dock_main_id);
            ImGuiID dock_left_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, nullptr, &dock_main_id);
            ImGuiID dock_left_top_id = ImGui::DockBuilderSplitNode(dock_left_id, ImGuiDir_Up, 0.4f, nullptr, &dock_left_bottom_id);

            // Dock Toolbar into the top split slot
            ImGui::DockBuilderDockWindow("Toolbar", dock_top_id);
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

        draw_imgui_tool_bar();

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

            if (current_fb_w != static_cast<uint32_t>(panel_w) || current_fb_h != static_cast<uint32_t>(panel_h)) {
                renderer.on_resize(panel_w, panel_h);
                if (camera_) {
                    const_cast<engine::Camera*>(camera_)->set_aspect_ratio(panel_w / panel_h);
                }
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

    void UILayer::on_event(engine::Event& event) {
        // engine::EventDispatcher dispatcher(event);
    }

    #ifdef ENABLE_IMGUI
    void UILayer::set_scene_context(const std::shared_ptr<engine::Scene>& current_scene) {
        active_scene_ = current_scene;
        hierarchy_.set_context(current_scene);
        object_inspector_.set_context(current_scene);
    }

    void UILayer::draw_imguizmo(engine::Entity selected_entity, const engine::Camera& camera, const ImVec2& viewport_pos, const ImVec2& viewport_size) {
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

    void UILayer::draw_imgui_menu_bar() {
        auto update_window_title = [](const std::string& path) {
            std::string scene_name = std::filesystem::path(path).stem().string();
            engine::Application::get().window().set_title("Engine - " + scene_name);
        };

        const bool is_ctrl = engine::Input::is_key_pressed(engine::Key::LEFT_CONTROL) ||
            engine::Input::is_key_pressed(engine::Key::RIGHT_CONTROL);

        auto handle_new_scene = [this] {
            engine::SceneSerializer serializer(active_scene_);
            serializer.deserialize("assets/scenes/clean.scene", *asset_manager_);
            engine::Application::get().window().set_title(engine::setting::FRAME_TITLE);
        };

        auto handle_load_scene = [this, update_window_title] {
            #ifdef _WIN32
            std::string selected_file = open_file_dialog();
            if (!selected_file.empty()) {
                engine::SceneSerializer serializer(active_scene_);
                serializer.deserialize(selected_file, *asset_manager_);
                if (std::filesystem::path(selected_file).filename() != "clean.scene") {
                    update_window_title(selected_file);
                }
            }
            #endif
        };

        auto handle_save_scene = [this, update_window_title] {
            #ifdef _WIN32
            std::string selected_file = save_file_dialog();
            if (!selected_file.empty()) {
                engine::SceneSerializer serializer(active_scene_);
                serializer.serialize(selected_file, *asset_manager_);
                if (std::filesystem::path(selected_file).filename() != "clean.scene") {
                    update_window_title(selected_file);
                }
            }
            #endif
        };

        if (is_ctrl && !engine::Input::is_keyboard_captured() && !ImGuizmo::IsUsing()) {
            if (engine::Input::is_key_pressed_once(engine::Key::N)) {
                handle_new_scene();
            }
            if (engine::Input::is_key_pressed_once(engine::Key::S)) {
                handle_save_scene();
            }
            if (engine::Input::is_key_pressed_once(engine::Key::O)) {
                handle_load_scene();
            }
        }

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                    handle_new_scene();
                }
                if (ImGui::MenuItem("Save Scene As", "Ctrl+S")) {
                    handle_save_scene();
                }
                if (ImGui::MenuItem("Load Scene", "Ctrl+O")) {
                    handle_load_scene();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

    // Todo: refactor to Toolbar.hpp
    void UILayer::draw_imgui_tool_bar() {
        if (!editor_layer_) return;

        const float slot_width = 36.0f;
        const float slot_height = 24.0f;
        const float spacing_x = ImGui::GetStyle().ItemSpacing.x;

        // 1. Determine minimum required slots
        size_t min_required_slots = 0;
        for (size_t i = 0; i < toolbar_slots_.size(); ++i) {
            if (toolbar_slots_[i].item.has_value()) {
                min_required_slots = i + 1;
            }
        }
        min_required_slots = std::max<size_t>(1, min_required_slots);

        // 2. Minimum window constraints
        const float window_padding_x = 16.0f;
        const float min_window_width = (min_required_slots * slot_width) +
            ((min_required_slots - 1) * spacing_x) +
            window_padding_x + 16.0f;

        ImGui::SetNextWindowSizeConstraints(ImVec2(min_window_width, 32.0f), ImVec2(FLT_MAX, FLT_MAX));

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 4));
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

        if (ImGui::Begin("Toolbar", nullptr, flags)) {
            const ImVec2 avail = ImGui::GetContentRegionAvail();

            size_t target_slot_count = static_cast<size_t>(
                std::floor((avail.x + spacing_x) / (slot_width + spacing_x))
            );
            target_slot_count = std::max(target_slot_count, min_required_slots);

            if (target_slot_count != toolbar_slots_.size()) {
                toolbar_slots_.resize(target_slot_count);
                for (size_t i = 0; i < toolbar_slots_.size(); ++i) {
                    if (toolbar_slots_[i].id.empty()) {
                        toolbar_slots_[i].id = "slot_" + std::to_string(i);
                    }
                }
            }

            // Center vertically
            const float center_y = ImGui::GetCursorPosY() + std::max(0.0f, (avail.y - slot_height) * 0.5f);
            ImGui::SetCursorPosY(center_y);

            // Right-align slots
            const float total_slots_width = (slot_width * static_cast<float>(toolbar_slots_.size())) +
                (spacing_x * static_cast<float>(toolbar_slots_.size() - 1));
            if (avail.x >= total_slots_width) {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + avail.x - total_slots_width);
            }

            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            for (size_t i = 0; i < toolbar_slots_.size(); ++i) {
                if (i > 0) ImGui::SameLine();

                auto& slot = toolbar_slots_[i];
                ImGui::PushID(slot.id.c_str());

                ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
                ImVec2 slot_max = ImVec2(cursor_pos.x + slot_width, cursor_pos.y + slot_height);

                // Render widget if occupied, otherwise invisible dummy space
                if (slot.item.has_value()) {
                    slot.item->render_func(ImVec2(slot_width, slot_height));

                    // Drag source
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                        ImGui::SetDragDropPayload("TOOLBAR_SLOT_SWAP", &i, sizeof(size_t));
                        ImGui::Text("Move: %s", slot.item->label.c_str());
                        ImGui::EndDragDropSource();
                    }
                }
                else {
                    // Invisible layout dummy
                    ImGui::Dummy(ImVec2(slot_width, slot_height));
                }

                // Drag target
                if (ImGui::BeginDragDropTarget()) {
                    // Draw custom yellow drop target box on empty slots when dragging over them
                    if (!slot.item.has_value()) {
                        draw_list->AddRect(cursor_pos, slot_max, IM_COL32(255, 200, 50, 230), 3.0f, 0, 1.5f);
                    }

                    // Accept payload while suppressing ImGui's default blue rectangle
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TOOLBAR_SLOT_SWAP", ImGuiDragDropFlags_AcceptNoDrawDefaultRect)) {
                        size_t source_idx = *static_cast<const size_t*>(payload->Data);
                        if (source_idx != i) {
                            std::swap(toolbar_slots_[source_idx].item, toolbar_slots_[i].item);
                        }
                    }
                    ImGui::EndDragDropTarget();
                }

                ImGui::PopID();
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    void UILayer::register_default_toolbar_items() {
        toolbar_slots_.clear();
        toolbar_slots_.resize(6); // 6 sichtbare Slots in der Toolbar

        for (size_t i = 0; i < toolbar_slots_.size(); ++i) {
            toolbar_slots_[i].id = "slot_" + std::to_string(i);
        }

        // Slot 0 mit Play/Stop belegen
        toolbar_slots_[0].item = ToolbarItem{
            "play_stop", "Play / Stop", 36.0f,
            [this](ImVec2 size) {
                const bool is_play = (editor_layer_->get_scene_state() == engine::SceneState::Play);
                const bool is_pause = (editor_layer_->get_scene_state() == engine::SceneState::Pause);

                if (is_play || is_pause) {
                    if (draw_icon_button("##StopBtn", draw_stop_icon, size)) editor_layer_->on_scene_stop();
                }
                else {
                    if (draw_icon_button("##PlayBtn", draw_play_icon, size)) editor_layer_->on_scene_play();
                }
            }
        };

        // Slot 1 mit Pause/Resume belegen
        toolbar_slots_[1].item = ToolbarItem{
            "pause_resume", "Pause / Resume", 36.0f,
            [this](ImVec2 size) {
                const bool is_play = (editor_layer_->get_scene_state() == engine::SceneState::Play);
                const bool is_pause = (editor_layer_->get_scene_state() == engine::SceneState::Pause);

                if (draw_icon_button("##PauseBtn", is_pause ? draw_play_icon : draw_pause_icon, size)) {
                    if (is_play) editor_layer_->on_scene_pause();
                    else if (is_pause) editor_layer_->on_scene_play();
                }
            }
        };
    }
    #endif
}
