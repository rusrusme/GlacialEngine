//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "ui/UIComponent.hpp"

namespace editor {
    class DockspaceLayout : public UIComponent {
    public:
        DockspaceLayout() : UIComponent("Layout") {}
        ~DockspaceLayout() override = default;

        void on_init(SystemContext& context) override;
        void on_render_ui(SystemContext& context) override;

        void load_config() override;
        void save_config() override;

        void reset_layout() { reset_layout_ = true; }

    private:
        bool reset_layout_{false};
        ImGuiID dockspace_id_{0};

        void build_default_layout(ImGuiID dockspace_id);
    };
}
