//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <filesystem>
#include <string>

#include "EditorSettings.hpp"
#include "core/Layer.hpp"
#include "rime/RimeTheme.hpp"
#include "rime/highlight/RimeHighlighter.hpp"

namespace editor {
    using namespace engine;

    class RimeSettings : public SettingsCategory {
    public:
        ~RimeSettings() override = default;

        bool change_size_on_scroll{true};
        float font_scale{1.0f};
        bool show_column_guide{true};
        int column_guide{120};

        RimeTheme theme{};

        [[nodiscard]] std::string_view name() const override { return "Rime"; }

        void on_render_ui(SystemContext& context) override;

        void load() override;
        void save() override;

        void scale_font(float delta_y);
    };

    struct TextDocument {
        std::filesystem::path filepath;
        std::string buffer;
        bool dirty = false;

        std::vector<HighlightedLine> highlighted_lines;
        bool highlight_dirty = true;
    };

    struct RimeContext {
        TextDocument active_document;
    };

    class RimeEditor : public Layer {
    public:
        RimeEditor() : Layer("Rime") {}
        ~RimeEditor() override = default;

        void on_attach(SystemContext& context) override;
        void on_detach(SystemContext& context) override;
        void on_update(SystemContext& context, float delta_time) override;
        void on_render(SystemContext& context, float delta_time) override;
        void on_event(SystemContext& context, Event& event) override;
    };
}
