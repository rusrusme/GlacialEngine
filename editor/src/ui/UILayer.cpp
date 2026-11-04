//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "UILayer.hpp"

namespace editor {
    void UILayer::on_attach(SystemContext& context) {
        ImguiLayer::on_attach(context);
    }

    void UILayer::on_detach(SystemContext& context) {
        ImguiLayer::on_detach(context);
    }

    void UILayer::on_update(SystemContext& context, float delta_time) {
        ImguiLayer::on_update(context, delta_time);
    }

    void UILayer::on_imgui_render(SystemContext& context, float delta_time) {
        ImguiLayer::on_imgui_render(context, delta_time);
    }
}
