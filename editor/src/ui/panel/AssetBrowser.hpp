//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "ui/UIComponent.hpp"

namespace editor {
    using namespace engine;

    class AssetBrowser : public UIComponent {
    public:
        AssetBrowser() : UIComponent("Asset Browser") {}
        ~AssetBrowser() override = default;

        void on_init(SystemContext& context) override;
        void on_render_ui(SystemContext& context) override;

        void load_config() override;
        void save_config() override;

        void toggle_visible() { visible_ = !visible_; }
        [[nodiscard]] bool visible() const { return visible_; }
        void set_visible(bool b) { visible_ = b; }

    private:
        bool visible_ = true;
    };
}
