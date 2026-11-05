//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "AssetBrowser.hpp"

#include "Config.hpp"

namespace editor {
    void AssetBrowser::on_init(SystemContext& context) {
        UIComponent::on_init(context);
    }

    void AssetBrowser::on_render_ui(SystemContext& context) {
    }

    void AssetBrowser::load_config() {
        visible_ = config::get_value("AssetBrowser_visible", visible_);
    }

    void AssetBrowser::save_config() {
        config::set_value("AssetBrowser_visible", visible_);
    }
}
