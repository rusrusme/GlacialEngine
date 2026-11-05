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
    }

    void Inspector::load_config() {
        visible_ = config::get_value("Inspector_visible", visible_);
    }

    void Inspector::save_config() {
        config::set_value("Inspector_visible", visible_);
    }
}
