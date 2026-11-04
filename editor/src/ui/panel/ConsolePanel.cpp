//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "ConsolePanel.hpp"

#include "core/Log.hpp"

namespace editor {
    void ConsolePanel::on_init(SystemContext& context) {
        UIComponent::on_init(context);
    }

    void ConsolePanel::on_render_ui(SystemContext& context) {
        ImGui::Begin("Console");

        if (ImGui::Button("Clear")) {
            Log::clear();
        }
        ImGui::SameLine();
        if (ImGui::Button("Copy All")) {
            std::string full_log;
            for (const auto& entry : Log::get_entries()) {
                full_log += entry.message + "\n";
            }
            ImGui::SetClipboardText(full_log.c_str());
        }

        ImGui::Separator();

        ImGui::BeginChild("LogScrollArea", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        static int selected_index = -1;
        const auto& entries = Log::get_entries();

        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(entries.size()));

        while (clipper.Step()) {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                const auto& entry = entries[i];
                ImVec4 color{1.0f, 1.0f, 1.0f, 1.0f};

                switch (entry.level) {
                    case LogLevel::Trace : color = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
                        break;
                    case LogLevel::Info : color = ImVec4(0.2f, 0.9f, 0.2f, 1.0f);
                        break;
                    case LogLevel::Warn : color = ImVec4(0.9f, 0.8f, 0.2f, 1.0f);
                        break;
                    case LogLevel::Error : color = ImVec4(0.9f, 0.2f, 0.2f, 1.0f);
                        break;
                    case LogLevel::Critical : color = ImVec4(1.0f, 0.1f, 0.1f, 1.0f);
                        break;
                }

                ImGui::PushID(i);

                ImGui::PushStyleColor(ImGuiCol_Text, color);
                const bool is_selected = (selected_index == i);
                if (ImGui::Selectable(entry.message.c_str(), is_selected)) {
                    selected_index = i;
                }
                ImGui::PopStyleColor();

                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Copy Line")) {
                        ImGui::SetClipboardText(entry.message.c_str());
                    }
                    ImGui::EndPopup();
                }

                ImGui::PopID();
            }
        }

        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }

        ImGui::EndChild();
        ImGui::End();
    }

    void ConsolePanel::load_config() {
        UIComponent::load_config();
    }

    void ConsolePanel::save_config() {
        UIComponent::save_config();
    }
}
