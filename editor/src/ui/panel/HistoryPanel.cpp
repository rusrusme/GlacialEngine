//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "HistoryPanel.hpp"

#include "Config.hpp"
#include "command/CommandHistory.hpp"
#include "core/SystemContext.hpp"

namespace editor {
    void HistoryPanel::on_render_ui(SystemContext& context) {
        if (!visible_) return;

        ImGui::SetNextWindowSize(ImVec2(320.0f, 480.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("History", &visible_);

        auto& history = context.get<CommandHistory>();

        // Top Status Bar: shows active nesting or unsaved changes
        if (history.nesting_level() > 0) {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Recording Transaction (Depth: %zu)...", history.nesting_level());
            ImGui::Separator();
        }

        // Action Buttons
        ImGui::BeginDisabled(!history.can_undo());
        if (ImGui::Button("Undo")) {
            history.undo(context);
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::BeginDisabled(!history.can_redo());
        if (ImGui::Button("Redo")) {
            history.redo(context);
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        if (ImGui::Button("Clear")) {
            history.clear();
        }

        ImGui::SameLine();
        ImGui::TextDisabled("| Steps: %zu", history.size());

        ImGui::Separator();

        // Transaction List
        ImGui::BeginChild("HistoryItems", ImVec2(0, 0), false);

        const size_t current_idx = history.current_index();
        const auto clean_idx = history.clean_index();

        // Initial / Scene Open State (Index 0)
        {
            ImGui::PushID(-1);
            const bool is_active_head = current_idx == 0;
            const bool is_saved = (clean_idx.has_value() && *clean_idx == 0);

            std::string label = "[Initial State]";
            if (is_saved) label += " (Saved)";

            if (is_active_head) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.9f, 0.4f, 1.0f));
            }
            else {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            }

            if (ImGui::Selectable(label.c_str(), is_active_head)) {
                history.jump_to(0, context);
            }
            ImGui::PopStyleColor();
            ImGui::PopID();
        }

        // All Recorded Transactions
        const auto& list = history.history();
        for (size_t i = 0; i < list.size(); ++i) {
            ImGui::PushID(static_cast<int>(i));

            const size_t item_pos = i + 1;
            const bool is_executed = (item_pos <= current_idx);
            const bool is_active_head = (item_pos == current_idx);
            const bool is_saved = (clean_idx.has_value() && *clean_idx == item_pos);

            // Active commands are bright; undone (redoable) commands are dimmed
            ImVec4 text_color = is_executed
                                    ? (is_active_head ? ImVec4(0.4f, 0.9f, 0.4f, 1.0f) : ImVec4(0.9f, 0.9f, 0.9f, 1.0f))
                                    : ImVec4(0.45f, 0.45f, 0.45f, 1.0f);

            ImGui::PushStyleColor(ImGuiCol_Text, text_color);

            std::string item_label = std::string(list[i]->name());
            if (is_saved) {
                item_label += "  [Saved]";
            }

            if (ImGui::Selectable(item_label.c_str(), is_active_head)) {
                history.jump_to(item_pos, context);
            }

            ImGui::PopStyleColor();
            ImGui::PopID();
        }

        ImGui::EndChild();
        ImGui::End();
    }

    void HistoryPanel::load_config() {
        visible_ = config::get_value("HistoryPanel_visible", false);
    }

    void HistoryPanel::save_config() {
        config::set_value("HistoryPanel_visible", visible_);
    }
}
