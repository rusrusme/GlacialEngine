//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <string_view>

#include "event/Shortcuts.hpp"

namespace engine {
    struct SystemContext;
}

namespace editor {
    using namespace engine;

    class UIComponent {
    public:
        UIComponent(std::string_view name = "") : name_(name) {}
        virtual ~UIComponent() = default;

        virtual void on_init(SystemContext& context) {}
        virtual void on_update(SystemContext& context, float delta_time) {}

        virtual void on_render_ui(SystemContext& context) = 0;

        virtual void load_config() {}
        virtual void save_config() {}

        [[nodiscard]] std::string_view name() const { return name_; }
        [[nodiscard]] virtual ShortcutContext shortcut_context() const { return ShortcutContext::Global; }

        void toggle_visible() { visible_ = !visible_; }
        [[nodiscard]] bool visible() const { return visible_; }
        void set_visible(bool b) { visible_ = b; }

    protected:
        bool visible_ = true;

    private:
        std::string_view name_;
    };
}
