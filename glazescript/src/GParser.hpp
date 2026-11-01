//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "Token.hpp"
#include "glaze/AST.hpp"

namespace glaze {
    class GParser {
    public:
        GParser(std::string file_name, std::vector<Token> tokens);

        [[nodiscard]] Program construct();

    private:
        // Declarations
        DeclarationPtr parse_declaration();
        StructDeclaration parse_struct_or_component(bool is_component);
        Field parse_field();
        EnumDeclaration parse_enum_declaration();
        SystemDeclaration parse_system_declaration();
        OnUpdateDeclaration parse_on_update_declaration();
        OnEventDeclaration parse_on_event_declaration();
        FunctionDeclaration parse_function_declaration();

        // Statements
        StatementPtr parse_statement();
        Block parse_block_statement();
        IfStatement parse_if_statement();
        SwitchStatement parse_switch_statement();
        StatementPtr parse_for_statement();
        ForCStatement parse_for_c_loop();
        SimpleStatementPtr parse_for_init();
        ForRangeStatement parse_for_range_loop();
        ForEcsStatement parse_for_ecs_loop(const Token& for_token);
        WhileStatement parse_while_statement();
        DoWhileStatement parse_do_while_statement();
        SimpleStatementPtr parse_simple_statement();
        SimpleStatementPtr parse_variable_declaration(bool is_mutable);
        SimpleStatementPtr parse_return_statement();
        SimpleStatementPtr parse_expression_or_assignment();

        // Expressions
        ExpressionPtr parse_expression();
        ExpressionPtr parse_logical_or();
        ExpressionPtr parse_logical_and();
        ExpressionPtr parse_equality();
        ExpressionPtr parse_relational();
        ExpressionPtr parse_additive();
        ExpressionPtr parse_multiplicative();
        ExpressionPtr parse_unary();
        ExpressionPtr parse_postfix();
        ExpressionPtr parse_primary();

        // Types
        TypePtr parse_type();

        // Lookahead Helpers
        [[nodiscard]] bool is_type_start() const;
        [[nodiscard]] size_t type_length() const;
        [[nodiscard]] size_t skip_generic_type_args(size_t idx) const;

        // Token Navigation
        template<typename... Types>
        bool match(Types... types) {
            return ((check(types) ? (advance(), true) : false) || ...);
        }

        [[nodiscard]] bool check(TokenType type) const;
        [[nodiscard]] const Token& previous() const;
        [[nodiscard]] const Token& peek() const;
        [[nodiscard]] const Token& peek_at(size_t offset) const;
        const Token& consume(TokenType type, std::string_view message);
        const Token& advance();
        [[nodiscard]] bool is_at_end() const;

        static [[nodiscard]] GlazeCompilerError syntax_error(const Token& token, std::string_view message) ;

    private:
        std::string file_name_;
        std::vector<Token> tokens_;
        size_t current_{0};
        bool in_system_hook_{false};
    };
}
