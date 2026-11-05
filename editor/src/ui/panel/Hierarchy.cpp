//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Hierarchy.hpp"

#include "Config.hpp"

namespace editor {
    void Hierarchy::on_render_ui(SystemContext& context) {
    }

    void Hierarchy::load_config() {
        visible_ = config::get_value("Hierarchy_visible", visible_);
    }

    void Hierarchy::save_config() {
        config::set_value("Hierarchy_visible", visible_);
    }
}
