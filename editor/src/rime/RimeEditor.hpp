//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <filesystem>
#include <string>

#include "Config.hpp"
#include "EditorSettings.hpp"
#include "core/Layer.hpp"

namespace editor {
    using namespace engine;

    class RimeSettings : public SettingsCategory {
    public:
        ~RimeSettings() override = default;

        bool change_size_on_scroll{true};
        float font_scale{1.0f};

        [[nodiscard]] std::string_view name() const override { return "Rime"; }

        void on_render_ui(SystemContext& context) override {
            ImGui::Spacing();
            ImGui::Checkbox("Change font size with Ctrl+Scrollwheel", &change_size_on_scroll);

            if (ImGui::SliderFloat("Font Scale", &font_scale, 0.5f, 3.0f, "%.2f")) {
                font_scale = std::clamp(font_scale, 0.5f, 3.0f);
            }
            if (ImGui::Button("Reset Font Scale")) {
                font_scale = 1.0f;
            }
        }

        void load() override {
            change_size_on_scroll = config::get_value("Rime_change_font_on_scroll", change_size_on_scroll);
            font_scale = config::get_value("Rime_font_scale", font_scale);
        }

        void save() override {
            config::set_value("Rime_change_font_on_scroll", change_size_on_scroll);
            config::set_value("Rime_font_scale", font_scale);
        }

        void scale_font(float delta_y) {
            font_scale += delta_y * 0.05f;
            font_scale = std::clamp(font_scale, 0.5f, 3.0f);
        }
    };

    struct TextDocument {
        std::filesystem::path filepath;
        std::string buffer;
        bool dirty = false;
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
