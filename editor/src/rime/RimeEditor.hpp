//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

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
        std::string title;
        std::string buffer;
        bool dirty = false;
        std::vector<HighlightedLine> highlighted_lines;
        bool highlight_dirty = true;

        // Zustand pro Tab
        int cursor_pos = 0;
        int last_cursor_pos = -1;
        double cursor_blink_time = 0.0;
        float scroll_x = 0.0f;
        float scroll_y = 0.0f;
    };

    struct RimeContext {
        std::vector<TextDocument> documents;
        int active_document_index = -1;
        int request_select_tab_index = -1;
        bool request_focus_panel = false;

        [[nodiscard]] TextDocument* active_document() {
            if (active_document_index >= 0 && active_document_index < static_cast<int>(documents.size())) {
                return &documents[static_cast<size_t>(active_document_index)];
            }
            return nullptr;
        }

        [[nodiscard]] const TextDocument* active_document() const {
            if (active_document_index >= 0 && active_document_index < static_cast<int>(documents.size())) {
                return &documents[static_cast<size_t>(active_document_index)];
            }
            return nullptr;
        }

        int open_document(const std::filesystem::path& path) {
            std::error_code ec;
            auto canonical = std::filesystem::weakly_canonical(path, ec);
            if (ec) canonical = path;

            for (size_t i = 0; i < documents.size(); ++i) {
                if (std::filesystem::equivalent(documents[i].filepath, canonical, ec)) {
                    active_document_index = static_cast<int>(i);
                    request_select_tab_index = static_cast<int>(i);
                    request_focus_panel = true;
                    return active_document_index;
                }
            }

            std::ifstream file(canonical);
            if (!file.is_open()) return -1;

            std::string content((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());

            auto& doc = documents.emplace_back();
            doc.filepath = canonical;
            doc.title = canonical.filename().string();
            doc.buffer = std::move(content);
            doc.dirty = false;
            doc.highlight_dirty = true;

            active_document_index = static_cast<int>(documents.size() - 1);
            request_select_tab_index = active_document_index;
            request_focus_panel = true;
            return active_document_index;
        }

        void close_document(int index) {
            if (index < 0 || index >= static_cast<int>(documents.size())) return;
            documents.erase(documents.begin() + index);
            if (documents.empty()) {
                active_document_index = -1;
                request_select_tab_index = -1;
            }
            else {
                active_document_index = std::clamp(active_document_index, 0, static_cast<int>(documents.size() - 1));
                request_select_tab_index = active_document_index;
            }
        }
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
