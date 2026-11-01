//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

namespace glaze {
    enum class TokenType {
        // Keywords
        IF, ELSE,
        SWITCH, CASE, DEFAULT,
        LET, VAR,
        FOR,
        DO, WHILE,
        BREAK, CONTINUE,
        RETURN,
        FN,
        STRUCT,
        ENUM,
        LIST,

        // ECS Keywords & Built-in Types
        ENTITY,    // ECS entity wrapper
        COMPONENT, // ECS components are implemented as structs
        SYSTEM,    // ECS system definition
        EXCLUDE,   // ECS view exclude clause
        ON_UPDATE, // System lifecycle hook
        ON_EVENT,  // System lifecycle hook
        EVENT,     // Built-in event type

        // Primitive Types
        VOID,
        BOOL,
        UINT8, INT8,
        UINT32, INT32,
        UINT64, INT64,
        FLOAT, DOUBLE,
        STRING, CHAR,

        // Math Types
        VEC2, VEC3, VEC4,
        MAT2, MAT3, MAT4,
        QUAT,

        // Literals & Identifiers
        IDENTIFIER,
        BOOL_LITERAL,
        INT_LITERAL,
        FLOAT_LITERAL,
        STRING_LITERAL,
        CHAR_LITERAL,

        // Arithmetic
        PLUS, MINUS, STAR, SLASH, PERCENT,
        PLUS_PLUS, MINUS_MINUS,
        PLUS_EQUALS, MINUS_EQUALS, STAR_EQUALS, SLASH_EQUALS, PERCENT_EQUALS,

        // Logic
        AND_AND, OR_OR,

        // Comparisons
        EQUALS, EQUALS_EQUALS, BANG, BANG_EQUALS,
        LESS, LESS_EQUALS, GREATER, GREATER_EQUALS,

        // Blocks
        LPAR, RPAR,         // ()
        LBRACE, RBRACE,     // {}
        LBRACKET, RBRACKET, // []

        // Separators & Control Flow
        COMMA, COLON, SEMICOLON,
        ARROW, // ->

        // Member Access
        DOT,

        // Comments
        HASH, // `#`

        EOF
    };
}
