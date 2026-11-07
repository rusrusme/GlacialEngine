//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Inspector.hpp"

#include "Config.hpp"

namespace editor {
    void Inspector::on_init(SystemContext& context) {
        UIComponent::on_init(context);
    }

    void Inspector::on_render_ui(SystemContext& context) {
        if (!visible_) return;

        ImGui::Begin("Inspector");

        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
            Shortcuts::set_active_context(shortcut_context());
        }

        // TODO: Inspector Widgets...

        ImGui::End();
    }

    void Inspector::load_config() {
        visible_ = config::get_value("Inspector_visible", visible_);
    }

    void Inspector::save_config() {
        config::set_value("Inspector_visible", visible_);
    }
}
