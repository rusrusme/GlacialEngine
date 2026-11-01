//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "GLexer.hpp"

#include <cerrno>
#include <charconv>
#include <cstdlib>

namespace glaze {
    const std::unordered_map<std::string_view, TokenType> GLexer::KEYWORDS = {
        {"if", TokenType::IF},
        {"else", TokenType::ELSE},
        {"for", TokenType::FOR},
        {"while", TokenType::WHILE},
        {"do", TokenType::DO},
        {"fn", TokenType::FN},
        {"struct", TokenType::STRUCT},
        {"enum", TokenType::ENUM},
        {"list", TokenType::LIST},

        {"entity", TokenType::ENTITY},
        {"component", TokenType::COMPONENT},
        {"system", TokenType::SYSTEM},
        {"exclude", TokenType::EXCLUDE},
        {"on_update", TokenType::ON_UPDATE},
        {"on_event", TokenType::ON_EVENT},
        {"event", TokenType::EVENT},

        {"return", TokenType::RETURN},
        {"break", TokenType::BREAK},
        {"continue", TokenType::CONTINUE},
        {"switch", TokenType::SWITCH},
        {"case", TokenType::CASE},
        {"default", TokenType::DEFAULT},

        {"let", TokenType::LET},
        {"var", TokenType::VAR},
        {"void", TokenType::VOID},

        {"bool", TokenType::BOOL},
        {"true", TokenType::BOOL_LITERAL},
        {"false", TokenType::BOOL_LITERAL},

        {"uint8", TokenType::UINT8},
        {"int8", TokenType::INT8},
        {"uint32", TokenType::UINT32},
        {"int32", TokenType::INT32},
        {"uint64", TokenType::UINT64},
        {"int64", TokenType::INT64},

        {"float", TokenType::FLOAT},
        {"double", TokenType::DOUBLE},

        {"string", TokenType::STRING},
        {"char", TokenType::CHAR},

        {"vec2", TokenType::VEC2},
        {"vec3", TokenType::VEC3},
        {"vec4", TokenType::VEC4},
        {"mat2", TokenType::MAT2},
        {"mat3", TokenType::MAT3},
        {"mat4", TokenType::MAT4},
        {"quat", TokenType::QUAT}
    };

    std::vector<Token> GLexer::tokenize(std::string_view source_code) {
        source_code_ = source_code;
        tokens_.clear();
        start_ = 0;
        current_ = 0;
        line_ = 1;
        column_ = 0;

        while (!is_at_end()) {
            start_ = current_;
            scan_token();
        }

        tokens_.emplace_back(TokenType::END_OF_FILE, "", std::monostate{}, line_, column_);
        return std::move(tokens_);
    }

    void GLexer::scan_token() {
        token_start_line_ = line_;
        token_start_column_ = column_ + 1;

        char c = advance();
        switch (c) {
            case '(' : add_token(TokenType::LPAR);
                break;
            case ')' : add_token(TokenType::RPAR);
                break;
            case '{' : add_token(TokenType::LBRACE);
                break;
            case '}' : add_token(TokenType::RBRACE);
                break;
            case '[' : add_token(TokenType::LBRACKET);
                break;
            case ']' : add_token(TokenType::RBRACKET);
                break;

            case ',' : add_token(TokenType::COMMA);
                break;
            case ':' : add_token(TokenType::COLON);
                break;
            case ';' : add_token(TokenType::SEMICOLON);
                break;
            case '.' : add_token(TokenType::DOT);
                break;

            case '+' : {
                if (match('+')) add_token(TokenType::PLUS_PLUS);
                else if (match('=')) add_token(TokenType::PLUS_EQUALS);
                else add_token(TokenType::PLUS);
                break;
            }
            case '-' : {
                if (match('-')) add_token(TokenType::MINUS_MINUS);
                else if (match('>')) add_token(TokenType::ARROW);
                else if (match('=')) add_token(TokenType::MINUS_EQUALS);
                else add_token(TokenType::MINUS);
                break;
            }
            case '*' : add_token(match('=') ? TokenType::STAR_EQUALS : TokenType::STAR);
                break;
            case '/' : add_token(match('=') ? TokenType::SLASH_EQUALS : TokenType::SLASH);
                break;
            case '%' : add_token(match('=') ? TokenType::PERCENT_EQUALS : TokenType::PERCENT);
                break;

            case '&' : {
                if (match('&')) {
                    add_token(TokenType::AND_AND);
                }
                else {
                    throw GlazeCompilerError("syntax error: unexpected character '&'", token_start_line_, token_start_column_);
                }
                break;
            }
            case '|' : {
                if (match('|')) {
                    add_token(TokenType::OR_OR);
                }
                else {
                    throw GlazeCompilerError("syntax error: unexpected character '|'", token_start_line_, token_start_column_);
                }
                break;
            }

            case '#' : {
                while (peek() != '\n' && !is_at_end()) {
                    advance();
                }
                break;
            }

            case '=' : add_token(match('=') ? TokenType::EQUALS_EQUALS : TokenType::EQUALS);
                break;
            case '!' : add_token(match('=') ? TokenType::BANG_EQUALS : TokenType::BANG);
                break;
            case '<' : add_token(match('=') ? TokenType::LESS_EQUALS : TokenType::LESS);
                break;
            case '>' : add_token(match('=') ? TokenType::GREATER_EQUALS : TokenType::GREATER);
                break;

            case '\n' : {
                line_++;
                column_ = 0;
                break;
            }
            case ' ' :
            case '\r' :
            case '\t' :
                break;

            case '\'' : character();
                break;
            case '"' : string();
                break;

            default : {
                if (is_digit(c)) {
                    number();
                }
                else if (is_alphabet(c)) {
                    identifier();
                }
                else {
                    throw GlazeCompilerError(std::format("syntax error: unexpected character '{}'", c), token_start_line_, token_start_column_);
                }
                break;
            }
        }
    }

    void GLexer::identifier() {
        while (is_alphabet_numeric(peek())) {
            advance();
        }

        std::string_view text = source_code_.substr(start_, current_ - start_);
        auto it = KEYWORDS.find(text);
        TokenType type = (it != KEYWORDS.end()) ? it->second : TokenType::IDENTIFIER;

        if (type == TokenType::BOOL_LITERAL) {
            add_token(type, text == "true");
        }
        else {
            add_token(type);
        }
    }

    void GLexer::number() {
        while (is_digit(peek())) {
            advance();
        }

        bool is_float = false;
        if (peek() == '.' && is_digit(peek_next())) {
            is_float = true;
            do {
                advance();
            }
            while (is_digit(peek()));
        }

        std::string_view text = source_code_.substr(start_, current_ - start_);
        if (is_float) {
            std::string text_str(text);
            char* end = nullptr;
            errno = 0;
            double val = std::strtod(text_str.c_str(), &end);
            if (errno == ERANGE || end != text_str.c_str() + text_str.size()) {
                throw GlazeCompilerError(std::format("syntax error: float literal out of range '{}'", text), token_start_line_, token_start_column_);
            }
            add_token(TokenType::FLOAT_LITERAL, val);
        }
        else {
            int64_t val = 0;
            auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), val);
            if (ec != std::errc{} || ptr != text.data() + text.size()) {
                throw GlazeCompilerError(std::format("syntax error: integer literal out of range '{}'", text), token_start_line_, token_start_column_);
            }
            add_token(TokenType::INT_LITERAL, val);
        }
    }

    void GLexer::string() {
        std::string result;

        while (peek() != '"' && !is_at_end()) {
            char c = advance();
            if (c == '\n') {
                line_++;
                column_ = 0;
                result.push_back(c);
            }
            else if (c == '\\') {
                if (is_at_end()) break;
                char escaped = advance();
                switch (escaped) {
                    case 'n' : result.push_back('\n');
                        break;
                    case 't' : result.push_back('\t');
                        break;
                    case 'r' : result.push_back('\r');
                        break;
                    case '0' : result.push_back('\0');
                        break;
                    case '"' : result.push_back('"');
                        break;
                    case '\\' : result.push_back('\\');
                        break;
                    default :
                        throw GlazeCompilerError(std::format("syntax error: invalid escape sequence '\\{}'", escaped), token_start_line_, token_start_column_);
                }
            }
            else {
                result.push_back(c);
            }
        }

        if (is_at_end()) {
            throw GlazeCompilerError("syntax error: unterminated string", token_start_line_, token_start_column_);
        }

        advance(); // Closing '"'
        add_token(TokenType::STRING_LITERAL, std::move(result));
    }

    void GLexer::character() {
        if (is_at_end()) {
            throw GlazeCompilerError("syntax error: unterminated character literal", token_start_line_, token_start_column_);
        }

        char value;
        char c = advance();

        if (c == '\'') {
            throw GlazeCompilerError("syntax error: empty character literal", token_start_line_, token_start_column_);
        }

        if (c == '\\') {
            if (is_at_end()) {
                throw GlazeCompilerError("syntax error: unterminated character literal", token_start_line_, token_start_column_);
            }
            char escaped = advance();
            switch (escaped) {
                case 'n' : value = '\n';
                    break;
                case 't' : value = '\t';
                    break;
                case 'r' : value = '\r';
                    break;
                case '0' : value = '\0';
                    break;
                case '\'' : value = '\'';
                    break;
                case '\\' : value = '\\';
                    break;
                default :
                    throw GlazeCompilerError(std::format("syntax error: invalid escape sequence '\\{}'", escaped), token_start_line_, token_start_column_);
            }
        }
        else if (c == '\n') {
            line_++;
            column_ = 0;
            value = c;
        }
        else {
            value = c;
        }

        if (peek() != '\'') {
            throw GlazeCompilerError("syntax error: unterminated character literal", token_start_line_, token_start_column_);
        }

        advance(); // Closing '\''
        add_token(TokenType::CHAR_LITERAL, value);
    }

    bool GLexer::match(char expected) {
        if (is_at_end() || source_code_[current_] != expected) return false;
        current_++;
        column_++;
        return true;
    }

    char GLexer::peek() const {
        if (is_at_end()) return '\0';
        return source_code_[current_];
    }

    char GLexer::peek_next() const {
        if (current_ + 1 >= source_code_.length()) return '\0';
        return source_code_[current_ + 1];
    }

    char GLexer::advance() {
        column_++;
        return source_code_[current_++];
    }

    bool GLexer::is_at_end() const {
        return current_ >= source_code_.length();
    }

    bool GLexer::is_digit(char c) {
        return c >= '0' && c <= '9';
    }

    bool GLexer::is_alphabet(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
    }

    bool GLexer::is_alphabet_numeric(char c) {
        return is_alphabet(c) || is_digit(c);
    }

    void GLexer::add_token(TokenType type) {
        add_token(type, std::monostate{});
    }

    void GLexer::add_token(TokenType type, TokenLiteral literal) {
        std::string_view text = source_code_.substr(start_, current_ - start_);
        tokens_.emplace_back(type, std::string(text), std::move(literal), token_start_line_, token_start_column_);
    }
}
