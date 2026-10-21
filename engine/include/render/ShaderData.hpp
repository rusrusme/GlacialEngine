//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <glm/glm.hpp>

namespace engine {
    struct alignas(16) FrameUniformData {
        glm::mat4 view_projection{1.0f};
        glm::mat4 view{1.0f};
        glm::mat4 projection{1.0f};
        glm::vec4 camera_position{0.0f};
        glm::vec2 viewport_size{0.0f};

        float time{0.0f};
        float delta_time{0.0f};
    };

    static_assert(sizeof(FrameUniformData) % 16 == 0, "FrameData size must be a multiple of 16 bytes!");

    struct alignas(16) GPUInstanceData {
        glm::mat4 model_matrix{1.0f};
        glm::vec4 color{1.0f}; // Albedo-Tint from MaterialComponent
    };

    static_assert(sizeof(GPUInstanceData) % 16 == 0, "GPUInstanceData size must be a multiple of 16 bytes!");

    struct alignas(16) DirectionalLightData {
        glm::vec4 direction{0.0f, -1.0f, 0.0f, 0.0f}; // xyz = Direction
        glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};      // rgb = Color, a = Intensity
    };

    static_assert(sizeof(DirectionalLightData) % 16 == 0, "DirectionalLightData size must be a multiple of 16 bytes!");
}
