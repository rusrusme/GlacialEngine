//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "RimeHighlighter.hpp"

#include <string>
#include <unordered_set>

namespace editor {
    namespace {
        // Matches GSyntaxHighlighter.KEYWORDS
        const std::unordered_set<std::string_view> s_keywords = {
            "struct", "component", "enum", "system", "fn",
            "on_update", "on_event", "let", "var",
            "if", "else", "switch", "case", "default",
            "for", "while", "do", "return", "break", "continue",
            "exclude", "true", "false"
        };

        // Matches GSyntaxHighlighter.TYPES
        const std::unordered_set<std::string_view> s_builtin_types = {
            "void", "uint8", "int8", "uint16", "int16",
            "uint32", "int32", "uint64", "int64",
            "float", "double", "char", "string", "bool",
            "vec2", "vec3", "vec4", "mat2", "mat3", "mat4", "quat",
            "entity", "event", "list"
        };

        // Matches SymbolTable::register_engine_builtins()
        const std::unordered_set<std::string_view> s_engine_builtin_types = {
            "BodyType", "AudioTriggerCondition",
            "LabelComponent", "IDComponent", "DisabledComponent",
            "TransformComponent", "MeshComponent", "MaterialComponent",
            "CameraComponent", "FlyCameraControllerComponent",
            "DirectionalLightComponent", "PointLightComponent",
            "RigidBodyComponent", "BoxColliderComponent",
            "UIComponent", "AudioSourceComponent", "AudioListenerComponent"
        };

        // Matches GSyntaxHighlighter.INTRINSICS & GBuiltinRegistry
        const std::unordered_set<std::string_view> s_intrinsics = {
            // Entity & List intrinsics
            "get", "add", "has", "remove", "remove_at", "clear",
            // Built-in GLM math functions
            "length", "normalize", "distance", "dot", "cross",
            "inverse", "transpose", "determinant",
            "slerp", "angleAxis",
            "abs", "sqrt", "min", "max", "clamp", "mix",
            "sin", "cos", "radians", "degrees"
        };

        struct RawToken {
            uint32_t line_idx;
            uint32_t col_offset;
            uint32_t length;
            std::string_view text;
            HighlightKind kind;
            bool is_identifier{false};
        };

        bool is_alpha(char c) {
            return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
        }

        bool is_digit(char c) {
            return c >= '0' && c <= '9';
        }

        bool is_alnum(char c) {
            return is_alpha(c) || is_digit(c);
        }
    }

    ImU32 RimeHighlighter::color_for(HighlightKind kind) noexcept {
        switch (kind) {
            case HighlightKind::Keyword : return IM_COL32(207, 142, 109, 255);     // JetBrains Orange
            case HighlightKind::Type : return IM_COL32(187, 119, 228, 255);        // Purple (Static Field / Type)
            case HighlightKind::Intrinsic : return IM_COL32(86, 182, 194, 255);    // Cyan (Built-in / Intrinsic)
            case HighlightKind::Function : return IM_COL32(97, 175, 239, 255);     // Soft Blue (Functions)
            case HighlightKind::Number : return IM_COL32(42, 172, 184, 255);       // Teal Number
            case HighlightKind::String : return IM_COL32(106, 171, 115, 255);      // Green String
            case HighlightKind::Operator : return IM_COL32(198, 204, 215, 255);    // Light Gray Operator
            case HighlightKind::Punctuation : return IM_COL32(171, 178, 191, 255); // Neutral Punctuation
            case HighlightKind::Comment : return IM_COL32(122, 126, 133, 255);     // Muted Gray Comment
            case HighlightKind::Default :
            default : return IM_COL32(212, 216, 224, 255); // Default Identifier Text
        }
    }

    std::vector<HighlightedLine> RimeHighlighter::tokenize(std::string_view source) {
        std::vector<HighlightedLine> lines;
        lines.push_back(HighlightedLine{.line_start_offset = 0, .line_length = 0, .spans = {}});

        std::vector<RawToken> raw_tokens;
        raw_tokens.reserve(source.size() / 4);

        size_t cur = 0;
        uint32_t current_line = 0;
        uint32_t line_start = 0;

        // ====================================================================
        // PASS 1: Fault-Tolerant Lexical Scan
        // ====================================================================
        while (cur < source.size()) {
            char c = source[cur];

            // Newline handling
            if (c == '\n') {
                lines[current_line].line_length = static_cast<uint32_t>(cur) - line_start;
                cur++;
                current_line++;
                line_start = static_cast<uint32_t>(cur);
                lines.push_back(HighlightedLine{.line_start_offset = line_start, .line_length = 0, .spans = {}});
                continue;
            }

            // Whitespace
            if (c == ' ' || c == '\t' || c == '\r') {
                cur++;
                continue;
            }

            const auto token_start = static_cast<uint32_t>(cur);
            const uint32_t col_offset = token_start - line_start;

            // Line Comments: '#' (or '//')
            if (c == '#' || (c == '/' && cur + 1 < source.size() && source[cur + 1] == '/')) {
                while (cur < source.size() && source[cur] != '\n' && source[cur] != '\r') {
                    cur++;
                }
                uint32_t len = static_cast<uint32_t>(cur) - token_start;
                raw_tokens.push_back({current_line, col_offset, len, source.substr(token_start, len), HighlightKind::Comment, false});
                continue;
            }

            // String or Character Literals (stops at newline so unclosed quotes don't break the file)
            if (c == '"' || c == '\'') {
                const char quote = c;
                cur++;
                while (cur < source.size() && source[cur] != '\n' && source[cur] != '\r') {
                    if (source[cur] == '\\' && cur + 1 < source.size() && source[cur + 1] != '\n') {
                        cur += 2;
                        continue;
                    }
                    if (source[cur] == quote) {
                        cur++;
                        break;
                    }
                    cur++;
                }
                uint32_t len = static_cast<uint32_t>(cur) - token_start;
                raw_tokens.push_back({current_line, col_offset, len, source.substr(token_start, len), HighlightKind::String, false});
                continue;
            }

            // Numeric Literals (Int & Float)
            if (is_digit(c)) {
                while (cur < source.size() && is_digit(source[cur])) {
                    cur++;
                }
                if (cur + 1 < source.size() && source[cur] == '.' && is_digit(source[cur + 1])) {
                    cur++;
                    while (cur < source.size() && is_digit(source[cur])) {
                        cur++;
                    }
                    if (cur < source.size() && (source[cur] == 'f' || source[cur] == 'F')) {
                        cur++;
                    }
                }
                uint32_t len = static_cast<uint32_t>(cur) - token_start;
                raw_tokens.push_back({current_line, col_offset, len, source.substr(token_start, len), HighlightKind::Number, false});
                continue;
            }

            // Identifiers, Keywords, Built-in Types, Intrinsics
            if (is_alpha(c)) {
                while (cur < source.size() && is_alnum(source[cur])) {
                    cur++;
                }
                uint32_t len = static_cast<uint32_t>(cur) - token_start;
                std::string_view text = source.substr(token_start, len);

                HighlightKind kind = HighlightKind::Default;
                bool is_id = false;

                if (s_keywords.contains(text)) {
                    kind = HighlightKind::Keyword;
                }
                else if (s_builtin_types.contains(text) || s_engine_builtin_types.contains(text)) {
                    kind = HighlightKind::Type;
                }
                else if (s_intrinsics.contains(text)) {
                    kind = HighlightKind::Intrinsic;
                }
                else {
                    is_id = true;
                }

                raw_tokens.push_back({current_line, col_offset, len, text, kind, is_id});
                continue;
            }

            // Multi-char Operators
            if (cur + 1 < source.size()) {
                std::string_view two = source.substr(cur, 2);
                if (two == "->" || two == "::" || two == "==" || two == "!=" ||
                    two == "<=" || two == ">=" || two == "&&" || two == "||" ||
                    two == "++" || two == "--" || two == "+=" || two == "-=" ||
                    two == "*=" || two == "/=" || two == "%=") {
                    cur += 2;
                    raw_tokens.push_back({current_line, col_offset, 2, two, HighlightKind::Operator, false});
                    continue;
                }
            }

            // Single-char Operators & Punctuation
            cur++;
            std::string_view one = source.substr(token_start, 1);
            HighlightKind punct_kind = HighlightKind::Punctuation;
            if (c == '=' || c == '<' || c == '>' || c == '!' ||
                c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
                c == '&' || c == '|') {
                punct_kind = HighlightKind::Operator;
            }
            raw_tokens.push_back({current_line, col_offset, 1, one, punct_kind, false});
        }

        lines[current_line].line_length = static_cast<uint32_t>(source.size()) - line_start;

        // ====================================================================
        // PASS 2: AST-Aware Contextual Classification (matches GAnnotator)
        // ====================================================================
        std::unordered_set<std::string_view> user_types;
        std::unordered_set<std::string_view> user_functions;

        // 2A. Collect top-level declarations & function names
        for (size_t i = 0; i < raw_tokens.size(); ++i) {
            const auto& tok = raw_tokens[i];
            if (tok.kind == HighlightKind::Keyword) {
                if ((tok.text == "struct" || tok.text == "component" || tok.text == "enum" || tok.text == "system") &&
                    i + 1 < raw_tokens.size() && raw_tokens[i + 1].is_identifier) {
                    user_types.insert(raw_tokens[i + 1].text);
                    raw_tokens[i + 1].kind = HighlightKind::Type;
                }
                else if (tok.text == "fn" && i + 2 < raw_tokens.size()) {
                    // fn <return_type> <name> (or fn list<T> <name>)
                    size_t idx = i + 1;
                    if (raw_tokens[idx].text == "list" && idx + 3 < raw_tokens.size() && raw_tokens[idx + 1].text == "<") {
                        int depth = 0;
                        ++idx;
                        while (idx < raw_tokens.size()) {
                            if (raw_tokens[idx].text == "<") depth++;
                            else if (raw_tokens[idx].text == ">") {
                                depth--;
                                if (depth == 0) {
                                    ++idx;
                                    break;
                                }
                            }
                            ++idx;
                        }
                    }
                    else {
                        ++idx;
                    }
                    if (idx < raw_tokens.size() && raw_tokens[idx].is_identifier) {
                        user_functions.insert(raw_tokens[idx].text);
                        raw_tokens[idx].kind = HighlightKind::Function;
                    }
                }
            }
        }

        // 2B. Classify usages, type positions, ECS query bindings, and function calls
        for (size_t i = 0; i < raw_tokens.size(); ++i) {
            auto& tok = raw_tokens[i];
            if (!tok.is_identifier) continue;

            if (user_types.contains(tok.text)) {
                tok.kind = HighlightKind::Type;
                continue;
            }
            if (user_functions.contains(tok.text)) {
                tok.kind = HighlightKind::Function;
                continue;
            }

            // Identifier followed by '(' is a function/constructor call
            if (i + 1 < raw_tokens.size() && raw_tokens[i + 1].text == "(") {
                tok.kind = HighlightKind::Function;
                continue;
            }

            // Identifier followed by another Identifier (e.g. `Velocity v` field, param, or ECS binding)
            // unless preceded by '.'
            const bool preceded_by_dot = (i > 0 && raw_tokens[i - 1].text == ".");
            if (!preceded_by_dot && i + 1 < raw_tokens.size() && raw_tokens[i + 1].is_identifier) {
                tok.kind = HighlightKind::Type;
                continue;
            }

            // Identifier after 'exclude' in ECS query: `exclude Frozen, Dead`
            if (i > 0 && raw_tokens[i - 1].text == "exclude") {
                tok.kind = HighlightKind::Type;
                continue;
            }
        }

        // Group tokens into lines
        for (const auto& tok : raw_tokens) {
            if (tok.line_idx < lines.size()) {
                lines[tok.line_idx].spans.push_back(HighlightSpan{
                    .column_offset = tok.col_offset,
                    .length        = tok.length,
                    .kind          = tok.kind
                });
            }
        }

        return lines;
    }

    std::vector<HighlightedLine> RimeHighlighter::tokenize_plain(std::string_view source) {
        std::vector<HighlightedLine> lines;
        lines.push_back(HighlightedLine{.line_start_offset = 0, .line_length = 0, .spans = {}});

        uint32_t current_line = 0;
        uint32_t line_start = 0;

        for (size_t cur = 0; cur < source.size(); ++cur) {
            if (source[cur] == '\n') {
                const uint32_t total_len = static_cast<uint32_t>(cur) - line_start;
                lines[current_line].line_length = total_len;

                uint32_t text_len = total_len;
                if (text_len > 0 && source[line_start + text_len - 1] == '\r') {
                    --text_len;
                }

                if (text_len > 0) {
                    lines[current_line].spans.push_back(HighlightSpan{
                        .column_offset = 0,
                        .length        = text_len,
                        .kind          = HighlightKind::Default
                    });
                }

                current_line++;
                line_start = static_cast<uint32_t>(cur + 1);
                lines.push_back(HighlightedLine{.line_start_offset = line_start, .line_length = 0, .spans = {}});
            }
        }

        const uint32_t total_len = static_cast<uint32_t>(source.size()) - line_start;
        lines[current_line].line_length = total_len;
        uint32_t text_len = total_len;
        if (text_len > 0 && source[line_start + text_len - 1] == '\r') {
            --text_len;
        }

        if (text_len > 0) {
            lines[current_line].spans.push_back(HighlightSpan{
                .column_offset = 0,
                .length        = text_len,
                .kind          = HighlightKind::Default
            });
        }

        return lines;
    }

    std::vector<HighlightedLine> RimeHighlighter::tokenize_json(std::string_view source) {
        std::vector<HighlightedLine> lines;
        lines.push_back(HighlightedLine{.line_start_offset = 0, .line_length = 0, .spans = {}});

        uint32_t current_line = 0;
        uint32_t line_start = 0;
        size_t cur = 0;

        while (cur < source.size()) {
            const char c = source[cur];

            if (c == '\n') {
                lines[current_line].line_length = static_cast<uint32_t>(cur) - line_start;
                cur++;
                current_line++;
                line_start = static_cast<uint32_t>(cur);
                lines.push_back(HighlightedLine{.line_start_offset = line_start, .line_length = 0, .spans = {}});
                continue;
            }

            if (c == ' ' || c == '\t' || c == '\r') {
                cur++;
                continue;
            }

            const auto token_start = static_cast<uint32_t>(cur);
            const uint32_t col_offset = token_start - line_start;

            if (c == '#' || (c == '/' && cur + 1 < source.size() && source[cur + 1] == '/')) {
                while (cur < source.size() && source[cur] != '\n' && source[cur] != '\r') {
                    cur++;
                }
                const uint32_t len = static_cast<uint32_t>(cur) - token_start;
                lines[current_line].spans.push_back(HighlightSpan{
                    .column_offset = col_offset,
                    .length        = len,
                    .kind          = HighlightKind::Comment
                });
                continue;
            }

            if (c == '/' && cur + 1 < source.size() && source[cur + 1] == '*') {
                cur += 2;
                while (cur + 1 < source.size() && !(source[cur] == '*' && source[cur + 1] == '/')) {
                    if (source[cur] == '\n') {
                        const uint32_t len = static_cast<uint32_t>(cur) - (line_start + col_offset);
                        if (len > 0) {
                            lines[current_line].spans.push_back(HighlightSpan{col_offset, len, HighlightKind::Comment});
                        }
                        lines[current_line].line_length = static_cast<uint32_t>(cur) - line_start;
                        cur++;
                        current_line++;
                        line_start = static_cast<uint32_t>(cur);
                        lines.push_back(HighlightedLine{.line_start_offset = line_start, .line_length = 0, .spans = {}});
                        continue;
                    }
                    cur++;
                }
                if (cur + 1 < source.size()) {
                    cur += 2;
                }
                const uint32_t len = static_cast<uint32_t>(cur) - (line_start + col_offset);
                if (len > 0) {
                    lines[current_line].spans.push_back(HighlightSpan{col_offset, len, HighlightKind::Comment});
                }
                continue;
            }

            if (c == '"') {
                cur++;
                while (cur < source.size() && source[cur] != '\n' && source[cur] != '\r') {
                    if (source[cur] == '\\' && cur + 1 < source.size() && source[cur + 1] != '\n') {
                        cur += 2;
                        continue;
                    }
                    if (source[cur] == '"') {
                        cur++;
                        break;
                    }
                    cur++;
                }
                const uint32_t len = static_cast<uint32_t>(cur) - token_start;

                size_t lookahead = cur;
                while (lookahead < source.size() && (source[lookahead] == ' ' || source[lookahead] == '\t' ||
                    source[lookahead] == '\r' || source[lookahead] == '\n')) {
                    lookahead++;
                }
                const bool is_key = (lookahead < source.size() && source[lookahead] == ':');

                lines[current_line].spans.push_back(HighlightSpan{
                    .column_offset = col_offset,
                    .length        = len,
                    .kind          = is_key ? HighlightKind::Type : HighlightKind::String
                });
                continue;
            }

            if (c == '-' || (c >= '0' && c <= '9')) {
                if (c == '-' && (cur + 1 >= source.size() || source[cur + 1] < '0' || source[cur + 1] > '9')) {
                    cur++;
                    lines[current_line].spans.push_back(HighlightSpan{col_offset, 1, HighlightKind::Operator});
                    continue;
                }

                if (c == '-') cur++;
                while (cur < source.size() && source[cur] >= '0' && source[cur] <= '9') {
                    cur++;
                }
                if (cur + 1 < source.size() && source[cur] == '.' && source[cur + 1] >= '0' && source[cur + 1] <= '9') {
                    cur++;
                    while (cur < source.size() && source[cur] >= '0' && source[cur] <= '9') {
                        cur++;
                    }
                }
                if (cur < source.size() && (source[cur] == 'e' || source[cur] == 'E')) {
                    cur++;
                    if (cur < source.size() && (source[cur] == '+' || source[cur] == '-')) {
                        cur++;
                    }
                    while (cur < source.size() && source[cur] >= '0' && source[cur] <= '9') {
                        cur++;
                    }
                }

                const uint32_t len = static_cast<uint32_t>(cur) - token_start;
                lines[current_line].spans.push_back(HighlightSpan{
                    .column_offset = col_offset,
                    .length        = len,
                    .kind          = HighlightKind::Number
                });
                continue;
            }

            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
                while (cur < source.size() && ((source[cur] >= 'a' && source[cur] <= 'z') ||
                    (source[cur] >= 'A' && source[cur] <= 'Z') ||
                    (source[cur] >= '0' && source[cur] <= '9') ||
                    source[cur] == '_')) {
                    cur++;
                }
                const uint32_t len = static_cast<uint32_t>(cur) - token_start;
                const std::string_view word = source.substr(token_start, len);

                HighlightKind kind = HighlightKind::Default;
                if (word == "true" || word == "false" || word == "null") {
                    kind = HighlightKind::Keyword;
                }

                lines[current_line].spans.push_back(HighlightSpan{
                    .column_offset = col_offset,
                    .length        = len,
                    .kind          = kind
                });
                continue;
            }

            cur++;
            const HighlightKind sym_kind = (c == ':') ? HighlightKind::Operator : HighlightKind::Punctuation;
            lines[current_line].spans.push_back(HighlightSpan{
                .column_offset = col_offset,
                .length        = 1,
                .kind          = sym_kind
            });
        }

        lines[current_line].line_length = static_cast<uint32_t>(source.size()) - line_start;
        return lines;
    }
}
