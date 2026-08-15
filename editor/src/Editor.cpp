//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Editor.hpp"

#include <glm/gtc/constants.hpp>

namespace editor {
    void Editor::on_init(engine::Renderer& renderer) {
        editor_layer_.on_attach(renderer);
        ImGuiIO& io = ImGui::GetIO();
        io.FontGlobalScale = 1.5f;
    }

    void Editor::on_update(engine::Renderer& renderer, GLFWwindow* window, engine::CameraController& camera_controller, const float delta_time) {
        total_time_ += delta_time;
        editor_layer_.on_update(renderer, window, camera_controller, delta_time);
    }

    void Editor::on_render(engine::Renderer& renderer) {
        editor_layer_.on_render(renderer);

        #ifdef ENABLE_IMGUI
        editor_layer_.on_imGuiRender(ImGui::GetIO().DeltaTime);
        #endif

        // --------------------------------
        // 1. Big cube grid test
        // --------------------------------
        cube_grid(renderer);

        // --------------------------------
        // 2. Orbiting spheres test
        // --------------------------------
        // orbiting_spheres(renderer);

        // --------------------------------
        // 3. Dynamic rotating ring test
        // --------------------------------
        // dynamic_ring(renderer);
    }

    void Editor::cube_grid(engine::Renderer& renderer) const {
        constexpr int grid_x = 30;
        constexpr int grid_y = 10;
        constexpr int grid_z = 30;
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

    void Editor::orbiting_spheres(engine::Renderer& renderer) const {
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

    void Editor::dynamic_ring(engine::Renderer& renderer) const {
        constexpr glm::vec3 ring_center = {0.0f, 35.0f, 0.0f};
        const glm::vec3 ring_normal = glm::normalize(glm::vec3(
            std::sin(total_time_),
            std::cos(total_time_ * 0.5f),
            0.0f
        ));
        renderer.draw_circle(ring_center, 12.0f, ring_normal, {0.0f, 1.0f, 0.8f}, 128);
    }
}

int main() {
    editor::Editor engine;
    return engine.run();
}
