//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "RimeEditorPanel.hpp"

#include "Config.hpp"
#include "core/SystemContext.hpp"
#include "rime/RimeEditor.hpp"

namespace editor {
    static int string_resize_callback(ImGuiInputTextCallbackData* data) {
        if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
            auto* str = static_cast<std::string*>(data->UserData);
            str->resize(data->BufTextLen);
            data->Buf = str->data();
        }
        return 0;
    }

    void RimeEditorPanel::on_render_ui(SystemContext& context) {
        if (!visible_) return;

        auto& rime = context.get<RimeContext>();
        auto& rime_settings = context.get<EditorSettings>().get<RimeSettings>();
        auto& doc = rime.active_document;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        if (ImGui::Begin("Rime")) {
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
                Shortcuts::set_active_context(shortcut_context());
            }

            const ImVec2 avail = ImGui::GetContentRegionAvail();
            if (doc.buffer.capacity() == 0) {
                doc.buffer.reserve(256);
            }

            constexpr ImGuiInputTextFlags input_flags = ImGuiInputTextFlags_AllowTabInput
                | ImGuiInputTextFlags_CallbackResize;

            ImFont* font = ImGui::GetFont();
            const float old_scale = font->Scale;
            font->Scale = rime_settings.font_scale;
            ImGui::PushFont(font);

            if (ImGui::InputTextMultiline(
                "##RimeEditorCanvas",
                doc.buffer.data(),
                doc.buffer.capacity() + 1,
                avail,
                input_flags,
                string_resize_callback,
                &doc.buffer)) {
                doc.dirty = true;
            }

            ImGui::PopFont();
            font->Scale = old_scale;
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }
}