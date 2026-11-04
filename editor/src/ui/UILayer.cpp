//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "UILayer.hpp"

namespace editor {
    void UILayer::on_attach(SystemContext& context) {

    }

    void UILayer::on_detach(SystemContext& context) {

    }

    void UILayer::on_update(SystemContext& context, float delta_time) {

    }

    void UILayer::on_imgui_render(SystemContext& context, float delta_time) {
        ImguiLayer::on_imgui_render(context, delta_time);
    }
}
