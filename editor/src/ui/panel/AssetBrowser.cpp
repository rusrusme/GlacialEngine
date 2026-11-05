//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "AssetBrowser.hpp"

#include "Config.hpp"

namespace editor {
    void AssetBrowser::on_init(SystemContext& context) {
    }

    void AssetBrowser::on_render_ui(SystemContext& context) {
        if (!visible_) return;

        ImGui::Begin("Asset Browser");

        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
            Shortcuts::set_active_context(shortcut_context());
        }

        // TODO: Asset Browser Content...

        ImGui::End();
    }

    void AssetBrowser::load_config() {
        visible_ = config::get_value("AssetBrowser_visible", visible_);
    }

    void AssetBrowser::save_config() {
        config::set_value("AssetBrowser_visible", visible_);
    }
}
