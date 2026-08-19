//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "EditorLayer.hpp"

#ifdef ENABLE_IMGUI
#include <imgui.h>
#endif

#include <core/Application.hpp>
#include <core/Config.hpp>
#include <core/Log.hpp>
#include <event/KeyEvents.hpp>
#include <event/MouseEvents.hpp>
#include <event/WindowEvents.hpp>
#include <input/Input.hpp>
#include <input/KeyCodes.hpp>
#include <input/MouseCodes.hpp>
#include <scene/ecs/Entity.hpp>

namespace editor {
    void EditorLayer::on_attach(engine::Renderer& renderer) {
        editor_grid_.init(renderer);

        active_scene_->on_init(renderer);

        camera_.apply_default_view();
        camera_controller_.reset_mouse_state();

        const uint32_t width = engine::Application::get().window().get_width();
        const uint32_t height = engine::Application::get().window().get_height();
        camera_.set_aspect_ratio(static_cast<float>(width) / static_cast<float>(height));
        camera_.set_projection_params(45.0f, 0.1f, 10000.0f);
    }

    void EditorLayer::on_detach() {
        if (active_scene_) {
            active_scene_->on_shutdown();
        }
    }

    void EditorLayer::on_update(const float delta_time) {
        if (engine::Input::is_key_pressed_once(engine::Key::F3)) {
            show_debug_panel_ = !show_debug_panel_;
        }
        if (active_scene_) {
            active_scene_->on_update(delta_time);
        }
        if (!engine::Input::is_mouse_captured()) {
            camera_controller_.update(camera_, delta_time);
        }
        else {
            camera_controller_.reset_mouse_state();
        }
        total_time_ += delta_time;

        // ----------------------------------------------
        // Ray-casted tile hovering for debug purposes
        // ----------------------------------------------
        const glm::vec2 mouse_pos = engine::Input::get_mouse_position();
        int window_w, window_h;
        glfwGetWindowSize(engine::Application::get().window().native(), &window_w, &window_h);
        const glm::vec2 viewport_size = {static_cast<float>(window_w), static_cast<float>(window_h)};
        const engine::Ray ray = camera_.screen_point_to_ray(mouse_pos, viewport_size);
        glm::vec3 hit_point;
        if (ray.intersect_plane_y(0.0f, hit_point)) {
            hover_tile_pos_ = glm::floor(hit_point);
            hover_tile_pos_.y = 0.0f;
            is_hovering_tile_ = true;
        }
        else {
            is_hovering_tile_ = false;
        }
        // ----------------------------------------------
    }

    void EditorLayer::on_render(engine::Renderer& renderer, const float delta_time) {
        if (show_grid_) {
            editor_grid_.render(renderer);
        }
        if (show_gizmo_) {
            // World Origin Gizmo (X = Red, Y = Green, Z = Blue)
            // renderer.draw_line({0, 0, 0}, {2, 0, 0}, {1, 0, 0});
            // renderer.draw_line({0, 0, 0}, {0, 2, 0}, {0, 1, 0});
            // renderer.draw_line({0, 0, 0}, {0, 0, 2}, {0, 0, 1});
        }
        if (is_hovering_tile_) {
            // renderer.draw_box(
            //     hover_tile_pos_,
            //     hover_tile_pos_ + glm::vec3(1.0f, 0.05f, 1.0f),
            //     glm::vec3(0.0f, 1.0f, 0.0f)
            // );
        }
        if (active_scene_) {
            active_scene_->on_render(renderer, camera_, delta_time);
        }

        // --------------------------------
        // Test objects
        // --------------------------------
        // cube_grid(renderer);
        // orbiting_spheres(renderer);
        // dynamic_ring(renderer);
        // wave_around_scene(renderer);
        // tree_trunks_around_pyramid(renderer);
    }

    void EditorLayer::on_imGuiRender(engine::Renderer& renderer, const float delta_time) {
        #ifdef ENABLE_IMGUI
        // ----------------------
        // Debug panel
        // ----------------------
        if (show_debug_panel_) {
            static float timer = 0.0f;
            static float fps_display = 0.0f;

            timer += delta_time;
            if (timer >= engine::setting::video::FPS_REFRESH_RATE_IN_SECONDS || fps_display == 0.0f) {
                fps_display = ImGui::GetIO().Framerate;
                timer = 0.0f;
            }
            ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
            ImGui::Begin("Debug panel");

            ImGui::Text("Performance");
            ImGui::Text("  FPS: %.1f", fps_display);
            ImGui::Text("  Frame Time: %.2f ms", 1000.0f / (fps_display > 0.0f ? fps_display : 1.0f));

            ImGui::Separator();
            ImGui::Text("Video");
            static bool vsync_enabled = engine::Application::get().window().is_vsync();
            if (ImGui::Checkbox("V-Sync", &vsync_enabled)) {
                glfwSwapInterval(vsync_enabled ? 1 : 0);
            }
            ImGui::Text("FPS Limit: %s", engine::setting::video::get_target_fps_str().c_str());
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::SliderInt("##", &engine::setting::video::target_fps_index, 0, engine::setting::video::num_presets - 1, engine::setting::video::get_target_fps_str().c_str());

            ImGui::Separator();
            ImGui::Text("Editor");
            bool grid_active = is_grid_enabled();
            if (ImGui::Checkbox("Show World Grid", &grid_active)) {
                set_grid_enabled(grid_active);
            }
            // bool gizmo_active = is_gizmo_enabled();
            // if (ImGui::Checkbox("Show Axis Gizmo", &gizmo_active)) {
            // set_gizmo_enabled(gizmo_active);
            // }
            ImGui::End();
        }

        // ----------------------
        // Shader library inspector
        // ----------------------
        if (ImGui::Begin("Shader Library Inspector")) {
            const auto& shaders = renderer.get_shader_library().get_shaders();

            ImGui::Text("Registered Shaders: %zu", shaders.size());
            ImGui::Separator();

            static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit;

            if (ImGui::BeginTable("ShaderLibraryTable", 3, flags)) {
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Program ID", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                ImGui::TableHeadersRow();

                for (const auto& [name, shader] : shaders) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", name.c_str());
                    ImGui::TableSetColumnIndex(1);
                    if (shader) {
                        ImGui::Text("%u", shader->get_renderer_id());
                    }
                    else {
                        ImGui::TextDisabled("-");
                    }
                    ImGui::TableSetColumnIndex(2);
                    if (shader) {
                        ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.2f, 1.0f), "Active");
                    }
                    else {
                        ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.2f, 1.0f), "Null");
                    }
                }
                ImGui::EndTable();
            }
        }
        ImGui::End();
        #endif
    }

    void EditorLayer::on_event(engine::Event& event) {
        engine::EventDispatcher dispatcher(event);

        dispatcher.dispatch<engine::KeyPressedEvent>([](const engine::KeyPressedEvent& e) {
            CLIENT_TRACE("Pressed Key: {0}", e.key_code());
            return false;
        });

        dispatcher.dispatch<engine::MouseButtonPressedEvent>([](const engine::MouseButtonPressedEvent& e) {
            CLIENT_TRACE("Clicked Button: {0}", e.button());
            return false;
        });

        // dispatcher.dispatch<engine::MouseMovedEvent>([](const engine::MouseMovedEvent& e) {
        //     CLIENT_TRACE("Moved Mouse to: {0}, {1}", e.x(), e.y());
        //     return false;
        // });

        dispatcher.dispatch<engine::WindowResizedEvent>([this](const engine::WindowResizedEvent& e) {
            if (e.height() > 0) {
                camera_.set_aspect_ratio(static_cast<float>(e.width()) / static_cast<float>(e.height()));
            }
            return false;
        });

        dispatcher.dispatch<engine::MouseScrolledEvent>([this](const engine::MouseScrolledEvent& e) {
            if (!engine::Input::is_mouse_captured()) {
                engine::CameraController::on_scroll(camera_, e.y_offset());
                return true;
            }
            return false;
        });
    }

    void EditorLayer::wave_around_scene(engine::Renderer& renderer) const {
        constexpr int segments = 256;

        const float wave_radius = 35.0f + std::sin(total_time_ * 1.5f) * 4.0f;
        const float wave_amplitude = 6.0f + std::sin(total_time_ * 2.5f) * 4.0f;
        const float wave_frequency = 8.0f + std::cos(total_time_ * 0.8f) * 3.0f;
        const float base_height = 18.0f + std::sin(total_time_ * 0.5f) * 5.0f;

        auto calculate_wave_point = [&](int i) -> glm::vec3 {
            const float angle = (static_cast<float>(i) / segments) * glm::two_pi<float>();

            const float y = base_height + std::sin(angle * wave_frequency + total_time_ * 3.0f) * wave_amplitude
                + std::cos(angle * wave_frequency * 0.5f - total_time_ * 2.0f) * (wave_amplitude * 0.3f);

            return {
                std::cos(angle) * wave_radius,
                y,
                std::sin(angle) * wave_radius
            };
        };

        for (int i = 0; i < segments; ++i) {
            const glm::vec3 p1 = calculate_wave_point(i);
            const glm::vec3 p2 = calculate_wave_point(i + 1);

            const float color_factor = static_cast<float>(i) / segments;
            const glm::vec3 color = {
                (std::sin(color_factor * glm::pi<float>() + total_time_ * 2.0f) + 1.0f) * 0.5f,
                (std::sin(color_factor * glm::two_pi<float>() + total_time_) + 1.0f) * 0.5f,
                (std::cos(color_factor * glm::pi<float>() + total_time_ * 3.0f) + 1.0f) * 0.5f
            };

            renderer.draw_line(p1, p2, color);
        }

        constexpr int total_cube_slots = 180;
        constexpr float floor_radius = 45.0f;
        constexpr float cube_half_size = 0.35f;
        constexpr float ground_y = cube_half_size;

        const bool active_phase = std::fmod(total_time_, 2.0f) < 1.0f;

        for (int i = 0; i < total_cube_slots; ++i) {
            const bool is_even_slot = (i % 2 == 0);

            if ((active_phase && is_even_slot) || (!active_phase && !is_even_slot)) {
                const float angle = (static_cast<float>(i) / total_cube_slots) * glm::two_pi<float>();

                const glm::vec3 pos = {
                    std::cos(angle) * floor_radius,
                    ground_y,
                    std::sin(angle) * floor_radius
                };

                const glm::vec3 min_pt = pos - glm::vec3(cube_half_size);
                const glm::vec3 max_pt = pos + glm::vec3(cube_half_size);

                const glm::vec3 cube_color = is_even_slot
                                                 ? glm::vec3(0.0f, 1.0f, 0.8f)
                                                 : glm::vec3(1.0f, 0.3f, 0.1f);

                renderer.draw_box(min_pt, max_pt, cube_color);
            }
        }

        constexpr float pyramid_half_base = 55.0f;
        constexpr float pyramid_height = 70.0f;
        constexpr float pyramid_base_y = 0.0f;

        constexpr std::array base_corners = {
            glm::vec3(-pyramid_half_base, pyramid_base_y, -pyramid_half_base),
            glm::vec3(pyramid_half_base, pyramid_base_y, -pyramid_half_base),
            glm::vec3(pyramid_half_base, pyramid_base_y, pyramid_half_base),
            glm::vec3(-pyramid_half_base, pyramid_base_y, pyramid_half_base)
        };

        constexpr glm::vec3 apex = {0.0f, pyramid_height, 0.0f};

        const float pulse = (std::sin(total_time_ * 2.0f) + 1.0f) * 0.5f;
        const glm::vec3 pyramid_color = {1.0f, 0.8f + pulse * 0.2f, 0.2f};

        for (size_t i = 0; i < 4; ++i) {
            renderer.draw_line(base_corners[i], base_corners[(i + 1) % 4], pyramid_color);
            renderer.draw_line(base_corners[i], apex, pyramid_color);
        }

        constexpr int vine_steps = 80;
        constexpr float vine_turns = 6.0f;
        constexpr float vine_radius = 2.5f;
        constexpr glm::vec3 vine_color = {0.1f, 0.9f, 0.3f};

        for (int c = 0; c < 4; ++c) {
            const glm::vec3 start = base_corners[c];
            const glm::vec3 edge_dir = apex - start;

            const glm::vec3 forward = glm::normalize(edge_dir);
            glm::vec3 up = {0.0f, 1.0f, 0.0f};
            if (std::abs(glm::dot(forward, up)) > 0.99f) {
                up = {1.0f, 0.0f, 0.0f};
            }
            const glm::vec3 right = glm::normalize(glm::cross(forward, up));
            up = glm::cross(right, forward);

            auto calculate_vine_point = [&](int step) -> glm::vec3 {
                const float t = static_cast<float>(step) / vine_steps;
                const glm::vec3 center_on_edge = start + edge_dir * t;

                const float angle = t * glm::two_pi<float>() * vine_turns + total_time_ * 2.0f;
                const float r = vine_radius * (1.0f - t * 0.7f);

                return center_on_edge + (right * std::cos(angle) + up * std::sin(angle)) * r;
            };

            for (int s = 0; s < vine_steps; ++s) {
                const glm::vec3 p1 = calculate_vine_point(s);
                const glm::vec3 p2 = calculate_vine_point(s + 1);
                renderer.draw_line(p1, p2, vine_color);
            }
        }

        constexpr glm::vec3 flower_center = apex + glm::vec3(0.0f, 1.0f, 0.0f);
        constexpr int num_petals = 8;
        constexpr float petal_length = 12.0f;
        constexpr float petal_width = 5.0f;

        constexpr glm::vec3 petal_color_1 = {0.95f, 0.05f, 0.15f};
        constexpr glm::vec3 petal_color_2 = {1.00f, 0.25f, 0.35f};
        constexpr glm::vec3 center_color = {1.00f, 0.85f, 0.10f};

        // Gelbe Blütenmitte
        constexpr float core_size = 3.0f;
        constexpr glm::vec3 core_top = flower_center + glm::vec3(0.0f, core_size * 1.5f, 0.0f);
        for (int i = 0; i < 6; ++i) {
            const float a1 = (static_cast<float>(i) / 6.0f) * glm::two_pi<float>() + total_time_ * 0.5f;
            const float a2 = (static_cast<float>((i + 1) % 6) / 6.0f) * glm::two_pi<float>() + total_time_ * 0.5f;

            const glm::vec3 p1 = flower_center + glm::vec3(std::cos(a1) * core_size, core_size * 0.5f, std::sin(a1) * core_size);
            const glm::vec3 p2 = flower_center + glm::vec3(std::cos(a2) * core_size, core_size * 0.5f, std::sin(a2) * core_size);

            renderer.draw_line(p1, p2, center_color);
            renderer.draw_line(p1, core_top, center_color);
            renderer.draw_line(p1, flower_center, center_color);
        }

        const float rotation_offset = total_time_ * 0.3f;
        for (int p = 0; p < num_petals; ++p) {
            const float petal_angle = (static_cast<float>(p) / num_petals) * glm::two_pi<float>() + rotation_offset;

            const glm::vec3 dir_out = {std::cos(petal_angle), 0.3f, std::sin(petal_angle)};
            const glm::vec3 dir_side = {-std::sin(petal_angle), 0.0f, std::cos(petal_angle)};

            constexpr glm::vec3 petal_base = flower_center;
            const glm::vec3 petal_mid = flower_center + dir_out * (petal_length * 0.5f) + glm::vec3(0.0f, 3.0f, 0.0f);
            const glm::vec3 petal_tip = flower_center + dir_out * petal_length + glm::vec3(0.0f, -1.0f, 0.0f);

            const glm::vec3 petal_left = petal_mid + dir_side * (petal_width * 0.5f);
            const glm::vec3 petal_right = petal_mid - dir_side * (petal_width * 0.5f);

            const glm::vec3& cur_color = (p % 2 == 0) ? petal_color_1 : petal_color_2;

            renderer.draw_line(petal_base, petal_left, cur_color);
            renderer.draw_line(petal_left, petal_tip, cur_color);
            renderer.draw_line(petal_tip, petal_right, cur_color);
            renderer.draw_line(petal_right, petal_base, cur_color);

            renderer.draw_line(petal_base, petal_tip, cur_color);
            renderer.draw_line(petal_left, petal_right, cur_color);
        }
    }

    void EditorLayer::tree_trunks_around_pyramid(engine::Renderer& renderer) const {
        constexpr int num_trees = 16;
        constexpr float circle_radius = 65.0f;
        constexpr float trunk_height = 22.0f;
        constexpr float trunk_radius = 1.8f;
        constexpr int trunk_segments = 8;
        constexpr int height_rings = 4;

        constexpr glm::vec3 wood_color = {0.45f, 0.25f, 0.1f};
        constexpr glm::vec3 branch_color = {0.35f, 0.20f, 0.08f};
        constexpr glm::vec3 leaf_color_1 = {0.1f, 0.8f, 0.2f};
        constexpr glm::vec3 leaf_color_2 = {0.05f, 0.5f, 0.15f};

        constexpr glm::vec3 petal_color_primary = {0.95f, 0.05f, 0.15f};
        constexpr glm::vec3 petal_color_secondary = {1.00f, 0.25f, 0.35f};
        constexpr glm::vec3 flower_center_color = {1.00f, 0.85f, 0.10f};

        for (int t = 0; t < num_trees; ++t) {
            const float base_angle = (static_cast<float>(t) / num_trees) * glm::two_pi<float>();

            const glm::vec3 trunk_center = {
                std::cos(base_angle) * circle_radius,
                0.0f,
                std::sin(base_angle) * circle_radius
            };

            auto get_trunk_point = [&](int ring, int seg) -> glm::vec3 {
                const float h_factor = static_cast<float>(ring) / height_rings;
                const float seg_angle = (static_cast<float>(seg) / trunk_segments) * glm::two_pi<float>();
                const float current_radius = trunk_radius * (1.0f - h_factor * 0.25f);

                return {
                    trunk_center.x + std::cos(seg_angle) * current_radius,
                    trunk_center.y + h_factor * trunk_height,
                    trunk_center.z + std::sin(seg_angle) * current_radius
                };
            };

            for (int ring = 0; ring <= height_rings; ++ring) {
                for (int seg = 0; seg < trunk_segments; ++seg) {
                    const glm::vec3 p1 = get_trunk_point(ring, seg);
                    const glm::vec3 p2 = get_trunk_point(ring, (seg + 1) % trunk_segments);

                    renderer.draw_line(p1, p2, wood_color);

                    if (ring < height_rings) {
                        const glm::vec3 p_above = get_trunk_point(ring + 1, seg);
                        renderer.draw_line(p1, p_above, wood_color);
                    }
                }
            }

            constexpr int branches_per_tree = 4;
            for (int b = 0; b < branches_per_tree; ++b) {
                const float attach_h = trunk_height * (0.55f + (static_cast<float>(b) / branches_per_tree) * 0.35f);
                const float branch_angle = base_angle + (static_cast<float>(b) - 1.5f) * 0.9f;

                const glm::vec3 branch_start = {
                    trunk_center.x + std::cos(branch_angle) * (trunk_radius * 0.6f),
                    attach_h,
                    trunk_center.z + std::sin(branch_angle) * (trunk_radius * 0.6f)
                };

                constexpr float branch_length = 8.0f;
                const glm::vec3 branch_end = branch_start + glm::vec3{
                    std::cos(branch_angle) * branch_length,
                    4.0f,
                    std::sin(branch_angle) * branch_length
                };

                auto draw_thick_branch = [&](const glm::vec3& start, const glm::vec3& end, float r_start, float r_end) {
                    const glm::vec3 dir = glm::normalize(end - start);
                    glm::vec3 up = {0.0f, 1.0f, 0.0f};
                    if (std::abs(glm::dot(dir, up)) > 0.99f) up = {1.0f, 0.0f, 0.0f};
                    const glm::vec3 right = glm::normalize(glm::cross(dir, up));
                    up = glm::cross(right, dir);

                    constexpr int segs = 4;
                    for (int i = 0; i < segs; ++i) {
                        const float a1 = (static_cast<float>(i) / segs) * glm::two_pi<float>();
                        const float a2 = (static_cast<float>((i + 1) % segs) / segs) * glm::two_pi<float>();

                        const glm::vec3 s1 = start + (right * std::cos(a1) + up * std::sin(a1)) * r_start;
                        const glm::vec3 s2 = start + (right * std::cos(a2) + up * std::sin(a2)) * r_start;
                        const glm::vec3 e1 = end + (right * std::cos(a1) + up * std::sin(a1)) * r_end;
                        const glm::vec3 e2 = end + (right * std::cos(a2) + up * std::sin(a2)) * r_end;

                        renderer.draw_line(s1, s2, branch_color);
                        renderer.draw_line(e1, e2, branch_color);
                        renderer.draw_line(s1, e1, branch_color);
                    }
                };

                draw_thick_branch(branch_start, branch_end, 0.9f, 0.5f);

                const glm::vec3 sub1_end = branch_end + glm::vec3{std::cos(branch_angle + 0.6f) * 4.0f, 2.5f, std::sin(branch_angle + 0.6f) * 4.0f};
                const glm::vec3 sub2_end = branch_end + glm::vec3{std::cos(branch_angle - 0.6f) * 4.0f, 2.0f, std::sin(branch_angle - 0.6f) * 4.0f};

                draw_thick_branch(branch_end, sub1_end, 0.5f, 0.25f);
                draw_thick_branch(branch_end, sub2_end, 0.5f, 0.25f);

                auto draw_leaf_cluster = [&](const glm::vec3& center, float size, const glm::vec3& color) {
                    const glm::vec3 top = center + glm::vec3(0, size, 0);
                    const glm::vec3 bottom = center - glm::vec3(0, size, 0);
                    const glm::vec3 left = center - glm::vec3(size, 0, 0);
                    const glm::vec3 right = center + glm::vec3(size, 0, 0);
                    const glm::vec3 front = center + glm::vec3(0, 0, size);
                    const glm::vec3 back = center - glm::vec3(0, 0, size);

                    renderer.draw_line(top, left, color);
                    renderer.draw_line(top, right, color);
                    renderer.draw_line(top, front, color);
                    renderer.draw_line(top, back, color);
                    renderer.draw_line(bottom, left, color);
                    renderer.draw_line(bottom, right, color);
                    renderer.draw_line(bottom, front, color);
                    renderer.draw_line(bottom, back, color);
                    renderer.draw_line(left, front, color);
                    renderer.draw_line(front, right, color);
                    renderer.draw_line(right, back, color);
                    renderer.draw_line(back, left, color);
                };

                draw_leaf_cluster(sub1_end, 2.2f, leaf_color_1);
                draw_leaf_cluster(sub2_end, 1.8f, leaf_color_2);
            }

            const glm::vec3 flower_center = trunk_center + glm::vec3(0.0f, trunk_height + 0.5f, 0.0f);
            constexpr int num_petals = 6;
            constexpr float petal_length = 4.8f;
            constexpr float petal_width = 2.2f;

            constexpr float core_size = 1.2f;
            const glm::vec3 core_top = flower_center + glm::vec3(0, core_size * 1.5f, 0);
            const glm::vec3 core_bot = flower_center;
            for (int i = 0; i < 4; ++i) {
                const float a1 = (static_cast<float>(i) / 4.0f) * glm::two_pi<float>();
                const float a2 = (static_cast<float>((i + 1) % 4) / 4.0f) * glm::two_pi<float>();

                const glm::vec3 p1 = flower_center + glm::vec3(std::cos(a1) * core_size, core_size * 0.7f, std::sin(a1) * core_size);
                const glm::vec3 p2 = flower_center + glm::vec3(std::cos(a2) * core_size, core_size * 0.7f, std::sin(a2) * core_size);

                renderer.draw_line(p1, p2, flower_center_color);
                renderer.draw_line(p1, core_top, flower_center_color);
                renderer.draw_line(p1, core_bot, flower_center_color);
            }

            for (int p = 0; p < num_petals; ++p) {
                const float petal_angle = (static_cast<float>(p) / num_petals) * glm::two_pi<float>();

                const glm::vec3 dir_out = {std::cos(petal_angle), 0.35f, std::sin(petal_angle)};
                const glm::vec3 dir_side = {-std::sin(petal_angle), 0.0f, std::cos(petal_angle)};

                const glm::vec3 petal_base = flower_center;
                const glm::vec3 petal_mid = flower_center + dir_out * (petal_length * 0.5f) + glm::vec3(0.0f, 1.2f, 0.0f);
                const glm::vec3 petal_tip = flower_center + dir_out * petal_length + glm::vec3(0.0f, -0.5f, 0.0f);

                const glm::vec3 petal_left = petal_mid + dir_side * (petal_width * 0.5f);
                const glm::vec3 petal_right = petal_mid - dir_side * (petal_width * 0.5f);

                const glm::vec3& cur_petal_color = (p % 2 == 0) ? petal_color_primary : petal_color_secondary;

                renderer.draw_line(petal_base, petal_left, cur_petal_color);
                renderer.draw_line(petal_left, petal_tip, cur_petal_color);
                renderer.draw_line(petal_tip, petal_right, cur_petal_color);
                renderer.draw_line(petal_right, petal_base, cur_petal_color);

                renderer.draw_line(petal_base, petal_tip, cur_petal_color);
                renderer.draw_line(petal_left, petal_right, cur_petal_color);
            }
        }
    }

    void EditorLayer::cube_grid(engine::Renderer& renderer) const {
        constexpr int grid_x = 10;
        constexpr int grid_y = 10;
        constexpr int grid_z = 10;
        constexpr float spacing = 2.5f;

        for (int x = 0; x < grid_x; ++x) {
            for (int y = 0; y < grid_y; ++y) {
                for (int z = 0; z < grid_z; ++z) {
                    const float pos_x = (static_cast<float>(x) - grid_x * 0.5f) * spacing;
                    const float pos_y = static_cast<float>(y) * spacing + 1.0f;
                    const float pos_z = (static_cast<float>(z) - grid_z * 0.5f) * spacing;

                    // Rainbow color based on position and time
                    const float r = (std::sin(pos_x * 0.1f + total_time_) + 1.0f) * 0.5f;
                    const float g = (std::sin(pos_y * 0.2f + total_time_ * 1.5f) + 1.0f) * 0.5f;
                    const float b = (std::cos(pos_z * 0.1f + total_time_) + 1.0f) * 0.5f;

                    const glm::vec3 min = {pos_x - 0.8f, pos_y - 0.8f, pos_z - 0.8f};
                    const glm::vec3 max = {pos_x + 0.8f, pos_y + 0.8f, pos_z + 0.8f};

                    renderer.draw_box(min, max, {r, g, b});
                }
            }
        }
    }

    void EditorLayer::orbiting_spheres(engine::Renderer& renderer) const {
        constexpr int num_spheres = 12;
        for (int i = 0; i < num_spheres; ++i) {
            const float angle = (static_cast<float>(i) / num_spheres) * glm::two_pi<float>() + total_time_;
            constexpr float radius_orbit = 25.0f;

            const glm::vec3 center = {
                std::cos(angle) * radius_orbit,
                15.0f + std::sin(total_time_ * 2.0f + static_cast<float>(i)) * 5.0f,
                std::sin(angle) * radius_orbit
            };
            renderer.draw_sphere(center, 3.0f, {1.0f, 0.2f, 0.2f}, 64);
        }
    }

    void EditorLayer::dynamic_ring(engine::Renderer& renderer) const {
        constexpr glm::vec3 ring_center = {0.0f, 35.0f, 0.0f};
        const glm::vec3 ring_normal = glm::normalize(glm::vec3(
            std::sin(total_time_),
            std::cos(total_time_ * 0.5f),
            0.0f
        ));
        renderer.draw_circle(ring_center, 12.0f, ring_normal, {0.0f, 1.0f, 0.8f}, 128);
    }
}
