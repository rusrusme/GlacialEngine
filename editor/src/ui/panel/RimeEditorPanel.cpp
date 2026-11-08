//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "RimeEditorPanel.hpp"

#include <cfloat>
#include <cmath>
#include <string_view>

#include "core/SystemContext.hpp"
#include "rime/RimeEditor.hpp"
#include "rime/highlight/RimeHighlighter.hpp"
#include "ui/UILayer.hpp"

namespace editor {
    static int rime_editor_callback(ImGuiInputTextCallbackData* data) {
        auto* user_data = static_cast<RimeEditorCallbackData*>(data->UserData);
        if (!user_data) return 0;

        if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
            user_data->buffer->resize(static_cast<size_t>(data->BufTextLen));
            data->Buf = user_data->buffer->data();
        }
        else if (data->EventFlag == ImGuiInputTextFlags_CallbackAlways) {
            if (user_data->cursor_pos) {
                *user_data->cursor_pos = data->CursorPos;
            }

            if (user_data->insert_spaces) {
                if (data->HasSelection()) {
                    data->DeleteChars(data->SelectionStart, data->SelectionEnd - data->SelectionStart);
                }
                data->InsertChars(data->CursorPos, "    ");
                user_data->insert_spaces = false;
            }
        }
        else if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter) {
            if (data->EventChar == '\t') {
                user_data->insert_spaces = true;
                return 1;
            }
        }
        return 0;
    }

    void RimeEditorPanel::on_render_ui(SystemContext& context) {
        if (!visible_) return;

        auto& rime = context.get<RimeContext>();

        if (rime.request_focus_panel) {
            visible_ = true;
            ImGui::SetNextWindowFocus();
            rime.request_focus_panel = false;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        if (ImGui::Begin("Rime", &visible_)) {
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
                Shortcuts::set_active_context(shortcut_context());
            }

            int doc_to_close = -1;

            if (ImGui::BeginTabBar("##RimeTabs", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs)) {
                for (int i = 0; i < static_cast<int>(rime.documents.size()); ++i) {
                    auto& doc = rime.documents[static_cast<size_t>(i)];

                    ImGuiTabItemFlags flags = ImGuiTabItemFlags_None;
                    if (doc.dirty) {
                        flags |= ImGuiTabItemFlags_UnsavedDocument;
                    }

                    if (rime.request_select_tab_index == i) {
                        flags |= ImGuiTabItemFlags_SetSelected;
                    }

                    bool tab_open = true;
                    const std::string tab_label = doc.title + "###doc_" + doc.filepath.string();

                    if (ImGui::BeginTabItem(tab_label.c_str(), &tab_open, flags)) {
                        rime.active_document_index = i;
                        ImGui::EndTabItem();
                    }

                    if (!tab_open) {
                        doc_to_close = i;
                    }
                }
                ImGui::EndTabBar();

                rime.request_select_tab_index = -1;
            }

            if (doc_to_close != -1) {
                rime.close_document(doc_to_close);
            }

            TextDocument* active_doc = rime.active_document();
            if (!active_doc) {
                render_empty_state();
            }
            else {
                const float status_bar_height = std::max(24.0f, ImGui::GetFrameHeight());
                const ImVec2 total_avail = ImGui::GetContentRegionAvail();
                const ImVec2 editor_avail(total_avail.x, std::max(0.0f, total_avail.y - status_bar_height));

                const ImVec2 start_pos = ImGui::GetCursorScreenPos();

                render_editor_view(context, *active_doc, editor_avail);
                render_status_bar(context, *active_doc, start_pos, total_avail.x, status_bar_height, editor_avail.y);
            }
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void RimeEditorPanel::render_empty_state() {
        const ImVec2 avail = ImGui::GetContentRegionAvail();
        const char* msg = "No script open. Double-click a .gscript in the Asset Browser to edit.";
        const ImVec2 text_size = ImGui::CalcTextSize(msg);

        const ImVec2 center_pos(
            ImGui::GetCursorPosX() + std::max(0.0f, (avail.x - text_size.x) * 0.5f),
            ImGui::GetCursorPosY() + std::max(0.0f, (avail.y - text_size.y) * 0.5f)
        );

        ImGui::SetCursorPos(center_pos);
        ImGui::TextDisabled("%s", msg);
    }

    void RimeEditorPanel::render_editor_view(SystemContext& context, TextDocument& doc, const ImVec2& avail) {
        auto& rime = context.get<RimeContext>();
        auto& rime_settings = context.get<EditorSettings>().get<RimeSettings>();
        const auto& theme = rime_settings.theme;

        if (doc.buffer.capacity() == 0) {
            doc.buffer.reserve(256);
        }

        auto update_tokens = [&doc] {
            const auto ext = doc.filepath.extension();
            if (ext == ".gscript") {
                doc.highlighted_lines = RimeHighlighter::tokenize(doc.buffer);
            }
            else if (ext == ".json") {
                doc.highlighted_lines = RimeHighlighter::tokenize_json(doc.buffer);
            }
            else {
                doc.highlighted_lines = RimeHighlighter::tokenize_plain(doc.buffer);
            }
            doc.highlight_dirty = false;
        };

        if (doc.highlight_dirty || doc.highlighted_lines.empty()) {
            update_tokens();
        }

        ImFont* font = UILayer::rime_font();
        if (!font) {
            font = ImGui::GetFont();
        }

        const float old_scale = font->Scale;
        font->Scale = rime_settings.font_scale;
        ImGui::PushFont(font);

        const auto line_count = static_cast<int64_t>(doc.highlighted_lines.size());
        int64_t temp = line_count;
        int digits = 1;
        while (temp >= 10) {
            temp /= 10;
            digits++;
        }
        digits = std::max(digits, theme.gutter_min_digits);

        const float char_width = ImGui::CalcTextSize("0").x;
        const float gutter_width = static_cast<float>(digits) * char_width + theme.gutter_pad_left + theme.gutter_pad_right;

        const ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
        const ImVec2 editor_pos(canvas_pos.x + gutter_width, canvas_pos.y);
        const ImVec2 editor_size(std::max(0.0f, avail.x - gutter_width), avail.y);

        const auto line_h = static_cast<double>(ImGui::GetTextLineHeight());
        const auto pad_x = static_cast<double>(ImGui::GetStyle().FramePadding.x);
        const auto pad_y = static_cast<double>(ImGui::GetStyle().FramePadding.y);

        ImGui::SetCursorScreenPos(editor_pos);

        RimeEditorCallbackData cb_data{
            .buffer        = &doc.buffer,
            .cursor_pos    = &doc.cursor_pos,
            .insert_spaces = false
        };

        constexpr ImGuiInputTextFlags input_flags = ImGuiInputTextFlags_AllowTabInput
            | ImGuiInputTextFlags_CallbackResize
            | ImGuiInputTextFlags_CallbackAlways
            | ImGuiInputTextFlags_CallbackCharFilter;

        const ImVec4 bg_color_vec = {theme.canvas_bg.r, theme.canvas_bg.g, theme.canvas_bg.b, theme.canvas_bg.a};
        const ImU32 bg_color_u32 = ImGui::ColorConvertFloat4ToU32(bg_color_vec);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, bg_color_vec);

        const std::string canvas_id = "##RimeEditorCanvas_" + std::to_string(rime.active_document_index);
        const bool text_changed = ImGui::InputTextMultiline(
            canvas_id.c_str(),
            doc.buffer.data(),
            doc.buffer.capacity() + 1,
            editor_size,
            input_flags,
            rime_editor_callback,
            &cb_data
        );
        const bool editor_active = ImGui::IsItemActive();
        ImGui::PopStyleColor(2);

        if (text_changed) {
            doc.dirty = true;
            update_tokens();
        }

        ImGuiWindow* parent_window = ImGui::GetCurrentWindow();
        ImGuiWindow* child_window = nullptr;
        for (auto* child : parent_window->DC.ChildWindows) {
            if (child && std::string_view(child->Name).find(canvas_id) != std::string_view::npos) {
                child_window = child;
                break;
            }
        }
        if (!child_window && !parent_window->DC.ChildWindows.empty()) {
            child_window = parent_window->DC.ChildWindows.back();
        }

        const ImVec2 mouse_pos = ImGui::GetMousePos();
        const bool gutter_hovered = (mouse_pos.x >= canvas_pos.x &&
            mouse_pos.x <= canvas_pos.x + gutter_width &&
            mouse_pos.y >= canvas_pos.y &&
            mouse_pos.y <= canvas_pos.y + avail.y);

        if (child_window) {
            if (gutter_hovered && ImGui::GetIO().MouseWheel != 0.0f) {
                child_window->Scroll.y = std::clamp(
                    child_window->Scroll.y - ImGui::GetIO().MouseWheel * static_cast<float>(line_h) * theme.scroll_wheel_speed,
                    0.0f,
                    child_window->ScrollMax.y
                );
            }
            doc.scroll_x = child_window->Scroll.x;
            doc.scroll_y = child_window->Scroll.y;
        }

        if (editor_active) {
            if (doc.cursor_pos != doc.last_cursor_pos || text_changed) {
                doc.last_cursor_pos = doc.cursor_pos;
                doc.cursor_blink_time = ImGui::GetTime();
            }
        }

        const auto cur_scroll_x = static_cast<double>(doc.scroll_x);
        const auto cur_scroll_y = static_cast<double>(doc.scroll_y);

        const int64_t first_line = std::max<int64_t>(0, static_cast<int64_t>((cur_scroll_y - pad_y) / line_h));
        const int64_t last_line = std::min<int64_t>(
            static_cast<int64_t>(doc.highlighted_lines.size()),
            static_cast<int64_t>((cur_scroll_y + static_cast<double>(avail.y) - pad_y) / line_h) + 1
        );

        ImDrawList* window_draw_list = ImGui::GetWindowDrawList();
        ImDrawList* text_draw_list = child_window ? child_window->DrawList : window_draw_list;
        const float font_size = ImGui::GetFontSize();

        const ImVec2 clip_min = child_window ? child_window->InnerClipRect.Min : editor_pos;
        const ImVec2 clip_max = child_window ? child_window->InnerClipRect.Max : ImVec2(editor_pos.x + editor_size.x, editor_pos.y + editor_size.y);

        text_draw_list->PushClipRect(clip_min, clip_max, true);

        const int guide_col = rime_settings.column_guide > 0 ? rime_settings.column_guide : 120;
        if (rime_settings.show_column_guide) {
            const float guide_x = std::floor(
                editor_pos.x + static_cast<float>(pad_x - cur_scroll_x) + (static_cast<float>(guide_col) * char_width)
            );

            if (guide_x >= clip_min.x && guide_x + 1.0f <= clip_max.x) {
                text_draw_list->AddRectFilled(
                    ImVec2(guide_x, clip_min.y),
                    ImVec2(guide_x + 1.0f, clip_max.y),
                    theme.guide_line_col
                );
            }
        }

        const char* buf_begin = doc.buffer.data();
        const auto buf_size = static_cast<uint32_t>(doc.buffer.size());

        for (int64_t i = first_line; i < last_line; ++i) {
            const auto& h_line = doc.highlighted_lines[static_cast<size_t>(i)];
            const double relative_y = (static_cast<double>(i) * line_h) - cur_scroll_y + pad_y;
            const float text_y = editor_pos.y + static_cast<float>(relative_y);
            const char* line_ptr = buf_begin + h_line.line_start_offset;

            for (const auto& span : h_line.spans) {
                if (h_line.line_start_offset + span.column_offset + span.length > buf_size) {
                    continue;
                }
                const char* tok_begin = line_ptr + span.column_offset;
                const char* tok_end = tok_begin + span.length;

                const float offset_x = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, line_ptr, tok_begin).x;
                const float text_x = editor_pos.x + static_cast<float>(pad_x - cur_scroll_x) + offset_x;

                text_draw_list->AddText(
                    font,
                    font_size,
                    ImVec2(text_x, text_y),
                    RimeHighlighter::color_for(span.kind),
                    tok_begin,
                    tok_end
                );
            }

            const auto u_cursor = static_cast<uint32_t>(std::clamp(doc.cursor_pos, 0, static_cast<int>(buf_size)));
            const uint32_t line_end_offset = h_line.line_start_offset + h_line.line_length;
            if (editor_active && u_cursor >= h_line.line_start_offset && u_cursor <= line_end_offset) {
                const double elapsed = ImGui::GetTime() - doc.cursor_blink_time;
                if (std::fmod(elapsed, static_cast<double>(theme.caret_blink_rate)) < theme.caret_blink_ratio) {
                    const char* caret_ptr = buf_begin + u_cursor;
                    const float caret_offset_x = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, line_ptr, caret_ptr).x;
                    const float caret_x = std::floor(editor_pos.x + static_cast<float>(pad_x - cur_scroll_x) + caret_offset_x);
                    text_draw_list->AddLine(
                        ImVec2(caret_x, text_y),
                        ImVec2(caret_x, text_y + static_cast<float>(line_h)),
                        theme.caret_col,
                        theme.caret_width
                    );
                }
            }
        }

        text_draw_list->PopClipRect();

        window_draw_list->AddRectFilled(
            canvas_pos,
            ImVec2(canvas_pos.x + gutter_width, canvas_pos.y + avail.y),
            bg_color_u32
        );

        window_draw_list->PushClipRect(
            canvas_pos,
            ImVec2(canvas_pos.x + gutter_width, canvas_pos.y + avail.y),
            true
        );

        for (int64_t i = first_line; i < last_line; ++i) {
            const int64_t line_num = i + 1;
            char num_str[32];
            std::snprintf(num_str, sizeof(num_str), "%lld", static_cast<long long>(line_num));

            const float text_w = ImGui::CalcTextSize(num_str).x;
            const float text_x = canvas_pos.x + gutter_width - theme.gutter_pad_right - text_w;

            const double relative_y = (static_cast<double>(i) * line_h) - cur_scroll_y + pad_y;
            const float text_y = canvas_pos.y + static_cast<float>(relative_y);

            window_draw_list->AddText(font, font_size, ImVec2(text_x, text_y), theme.line_number_col, num_str);
        }

        window_draw_list->PopClipRect();

        ImGui::PopFont();
        font->Scale = old_scale;
    }

    void RimeEditorPanel::render_status_bar(SystemContext& context, const TextDocument& doc, const ImVec2& start_pos, float width, float bar_height, float editor_height) {
        int64_t current_line = 0;
        int64_t current_col = 0;
        const uint32_t u_cursor = static_cast<uint32_t>(std::clamp(doc.cursor_pos, 0, static_cast<int>(doc.buffer.size())));

        for (size_t i = 0; i < doc.highlighted_lines.size(); ++i) {
            const auto& hl = doc.highlighted_lines[i];
            const uint32_t line_end = hl.line_start_offset + hl.line_length;
            if (u_cursor >= hl.line_start_offset && (u_cursor <= line_end || i == doc.highlighted_lines.size() - 1)) {
                current_line = static_cast<int64_t>(i);
                current_col = static_cast<int64_t>(u_cursor - hl.line_start_offset);
                break;
            }
        }

        const float bar_y = start_pos.y + editor_height;
        const ImVec2 bar_min(start_pos.x, bar_y);
        const ImVec2 bar_max(start_pos.x + width, bar_y + bar_height);

        auto& rime_settings = context.get<EditorSettings>().get<RimeSettings>();
        const auto& theme = rime_settings.theme;
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        draw_list->AddLine(bar_min, ImVec2(bar_max.x, bar_min.y), theme.gutter_divider_col, 1.0f);

        const float text_h = ImGui::GetTextLineHeight();
        const float text_y = bar_y + std::floor((bar_height - text_h) * 0.5f);
        ImGui::SetCursorScreenPos(ImVec2(bar_min.x + 8.0f, text_y));

        std::string_view file_type = "Plain Text";
        const auto ext = doc.filepath.extension();
        if (ext == ".gscript") file_type = "GlazeScript";
        else if (ext == ".json") file_type = "JSON";
        else if (ext == ".txt") file_type = "Text";

        ImGui::TextDisabled("Ln %lld, Col %lld  |  %.*s  |  UTF-8",
                            static_cast<long long>(current_line + 1),
                            static_cast<long long>(current_col + 1),
                            static_cast<int>(file_type.size()), file_type.data());

        if (doc.dirty) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.2f, 1.0f), "[Modified]");
        }

        ImGui::SameLine();
        ImGui::TextDisabled("|  %s", doc.filepath.string().c_str());
    }
}
