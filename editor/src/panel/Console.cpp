//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Console.hpp"

namespace editor {
    #ifdef ENABLE_IMGUI
    void Console::render_imgui() {
        ImGui::Begin("Console");

        if (ImGui::Button("Clear")) {
            engine::Log::clear();
        }

        ImGui::Separator();

        ImGui::BeginChild("LogScrollArea", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        for (const auto& entry : engine::Log::get_entries()) {
            ImVec4 color{1.0f, 1.0f, 1.0f, 1.0f};

            switch (entry.level) {
                case engine::LogLevel::Trace :
                    color = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
                    break;
                case engine::LogLevel::Info :
                    color = ImVec4(0.2f, 0.9f, 0.2f, 1.0f);
                    break;
                case engine::LogLevel::Warn :
                    color = ImVec4(0.9f, 0.8f, 0.2f, 1.0f);
                    break;
                case engine::LogLevel::Error :
                    color = ImVec4(0.9f, 0.2f, 0.2f, 1.0f);
                    break;
                case engine::LogLevel::Critical :
                    color = ImVec4(1.0f, 0.1f, 0.1f, 1.0f);
                    break;
            }
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::TextUnformatted(entry.message.c_str());
            ImGui::PopStyleColor();
        }
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();
        ImGui::End();
    }
    #endif
}
