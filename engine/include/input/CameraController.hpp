//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <render/Camera.hpp>

namespace engine {
    class CameraController {
    public:
        void reset_mouse_state();

        void update(Camera& camera, float delta_time);

        static void on_scroll(Camera& camera, double y_offset);

    private:
        double last_x_ = 0.0;
        double last_y_ = 0.0;
        bool is_first_frame_ = true;

        void update_keyboard(Camera& camera, float delta_time);

        void update_mouse(Camera& camera);
    };
}
