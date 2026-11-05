//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "ui/UIComponent.hpp"

namespace editor {
    class Menubar : public UIComponent {
    public:
        Menubar() : UIComponent("Menubar") {}
        ~Menubar() override = default;

        void on_init(SystemContext& context) override;
        void on_render_ui(SystemContext& context) override;

    private:
        void file_menu(SystemContext& context);
        void edit_menu(SystemContext& context);
        void window_menu(SystemContext& context);
        void help_menu(SystemContext& context);
    };
}
