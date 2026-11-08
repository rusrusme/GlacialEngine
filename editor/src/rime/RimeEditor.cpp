//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "RimeEditor.hpp"

#include "Config.hpp"

namespace editor {
    void RimeSettings::on_render_ui(SystemContext& context) {
        ImGui::Spacing();
        ImGui::Checkbox("Change font size with Ctrl+Scrollwheel", &change_size_on_scroll);
        if (ImGui::SliderFloat("Font Scale", &font_scale, 0.8f, 4.0f, "%.1f")) {
            font_scale = std::clamp(font_scale, 0.8f, 4.0f);
        }
        if (ImGui::Button("Reset Font Scale")) {
            font_scale = 1.0f;
        }
        ImGui::Separator();
        ImGui::Checkbox("Show Right Margin Guide", &show_column_guide);
        ImGui::SliderInt("Column Limit", &column_guide, 40, 200);
    }

    void RimeSettings::load() {
        change_size_on_scroll = config::get_value("Rime_change_font_on_scroll", change_size_on_scroll);
        font_scale = config::get_value("Rime_font_scale", font_scale);
        show_column_guide = config::get_value("Rime_show_column_guide", show_column_guide);
        column_guide = config::get_value("Rime_column_guide", column_guide);
    }

    void RimeSettings::save() {
        config::set_value("Rime_change_font_on_scroll", change_size_on_scroll);
        config::set_value("Rime_font_scale", font_scale);
        config::set_value("Rime_show_column_guide", show_column_guide);
        config::set_value("Rime_column_guide", column_guide);
    }

    void RimeSettings::scale_font(float delta_y) {
        font_scale += delta_y * 0.1f;
        font_scale = std::clamp(font_scale, 0.8f, 4.0f);
    }

    void RimeEditor::on_attach(SystemContext& context) {
        Layer::on_attach(context);
    }

    void RimeEditor::on_detach(SystemContext& context) {
        Layer::on_detach(context);
    }

    void RimeEditor::on_update(SystemContext& context, float delta_time) {
        Layer::on_update(context, delta_time);
    }

    void RimeEditor::on_render(SystemContext& context, float delta_time) {
        Layer::on_render(context, delta_time);
    }

    void RimeEditor::on_event(SystemContext& context, Event& event) {
        Layer::on_event(context, event);
    }
}
