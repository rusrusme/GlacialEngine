//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <array>

#include "core/Layer.hpp"
#include "debug/ImguiLayer.hpp"
#include "layout/Menubar.hpp"
#include "layout/Toolbar.hpp"
#include "panel/AssetBrowser.hpp"
#include "panel/ConsolePanel.hpp"
#include "panel/Hierarchy.hpp"
#include "panel/Inspector.hpp"
#include "ui/layout/DockspaceLayout.hpp"
#include "ui/panel/DebugPanel.hpp"
#include "ui/panel/HistoryPanel.hpp"
#include "ui/panel/ViewportPanel.hpp"
#include "window/SettingsWindow.hpp"

namespace editor {
    using namespace engine;

    class UILayer : public ImguiLayer {
    public:
        UILayer() : ImguiLayer("Editor UI Layer") {}
        ~UILayer() override = default;

        void on_attach(SystemContext& context) override;
        void on_detach(SystemContext& context) override;
        void on_update(SystemContext& context, float delta_time) override;
        void on_event(SystemContext& context, Event& event) override;

    protected:
        void on_imgui_render(SystemContext& context, float delta_time) override;

    private:
        std::array<UIComponent*, 11> ui_components_{};

        DockspaceLayout layout_;
        Menubar menubar_;
        Toolbar toolbar_;
        SettingsWindow settings_window_;
        Inspector inspector_panel_;
        Hierarchy hierarchy_panel_;
        AssetBrowser asset_browser_panel_;
        ConsolePanel console_panel_;
        DebugPanel debug_panel_;
        ViewportPanel viewport_panel_;
        HistoryPanel history_panel_;
    };
}
