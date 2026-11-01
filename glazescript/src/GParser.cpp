//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "GParser.hpp"

namespace glaze {
    GParser::GParser(std::string file_name, std::vector<Token> tokens)
        : file_name_(std::move(file_name)), tokens_(std::move(tokens)) {}

    Program GParser::construct() {
        std::vector<DeclarationPtr> declarations;
        while (!is_at_end()) {
            declarations.push_back(parse_declaration());
        }
        return Program{std::move(file_name_), std::move(declarations)};
    }

    // DECLARATIONS

    DeclarationPtr GParser::parse_declaration() {
        if (match(TokenType::STRUCT)) return make_decl<StructDeclaration>(parse_struct_or_component(false));
        if (match(TokenType::COMPONENT)) return make_decl<StructDeclaration>(parse_struct_or_component(true));
        if (match(TokenType::ENUM)) return make_decl<EnumDeclaration>(parse_enum_declaration());
        if (match(TokenType::SYSTEM)) return make_decl<SystemDeclaration>(parse_system_declaration());
        if (match(TokenType::FN)) return make_decl<FunctionDeclaration>(parse_function_declaration());

        throw syntax_error(peek(), "expected top-level declaration ('struct', 'component', 'enum', 'system', or 'fn')");
    }

    StructDeclaration GParser::parse_struct_or_component(bool is_component) {
        std::string_view kind = is_component ? "component" : "struct";
        const Token& name = consume(TokenType::IDENTIFIER, std::format("expected {} name", kind));
        consume(TokenType::LBRACE, std::format("expected '{{' after {} name", kind));

        std::vector<Field> fields;
        while (!check(TokenType::RBRACE) && !is_at_end()) {
            fields.push_back(parse_field());
        }
        consume(TokenType::RBRACE, std::format("expected '}}' after {} body", kind));
        return StructDeclaration{name.lexeme, std::move(fields), is_component};
    }

    Field GParser::parse_field() {
        TypePtr type = parse_type();
        const Token& name = consume(TokenType::IDENTIFIER, "expected field name");
        ExpressionPtr initializer = nullptr;

        if (match(TokenType::EQUALS)) {
            initializer = parse_expression();
        }
        match(TokenType::SEMICOLON);
        return Field{std::move(type), name.lexeme, std::move(initializer)};
    }

    EnumDeclaration GParser::parse_enum_declaration() {
        const Token& name = consume(TokenType::IDENTIFIER, "expected enum name");
        consume(TokenType::LBRACE, "expected '{' after enum name");

        std::vector<EnumMember> members;
        while (!check(TokenType::RBRACE) && !is_at_end()) {
            const Token& member_name = consume(TokenType::IDENTIFIER, "expected enum member name");
            std::optional<int64_t> value;

            if (match(TokenType::EQUALS)) {
                const Token& val_token = consume(TokenType::INT_LITERAL, "expected integer literal for enum value");
                value = std::get<int64_t>(val_token.literal);
            }
            members.push_back(EnumMember{member_name.lexeme, value});

            if (!match(TokenType::COMMA)) {
                match(TokenType::SEMICOLON);
            }
        }
        consume(TokenType::RBRACE, "expected '}' after enum body");
        return EnumDeclaration{name.lexeme, std::move(members)};
    }

    SystemDeclaration GParser::parse_system_declaration() {
        const Token& system_name = consume(TokenType::IDENTIFIER, "expected system name after 'system'");
        consume(TokenType::LBRACE, "expected '{' after system name");

        std::optional<OnUpdateDeclaration> on_update;
        std::optional<OnEventDeclaration> on_event;
        std::vector<FunctionDeclaration> functions;

        while (!check(TokenType::RBRACE) && !is_at_end()) {
            if (match(TokenType::ON_UPDATE)) {
                if (on_update.has_value()) {
                    throw syntax_error(previous(), std::format("system '{}' can only define one 'on_update'", system_name.lexeme));
                }
                on_update = parse_on_update_declaration();
            }
            else if (match(TokenType::ON_EVENT)) {
                if (on_event.has_value()) {
                    throw syntax_error(previous(), std::format("system '{}' can only define one 'on_event'", system_name.lexeme));
                }
                on_event = parse_on_event_declaration();
            }
            else if (match(TokenType::FN)) {
                functions.push_back(parse_function_declaration());
            }
            else {
                throw syntax_error(peek(), "expected 'on_update', 'on_event', or 'fn' inside system");
            }
        }

        consume(TokenType::RBRACE, "expected '}' after system body");

        if (!on_update.has_value() && !on_event.has_value()) {
            throw syntax_error(system_name, std::format("system '{}' must define at least 'on_update' or 'on_event'", system_name.lexeme));
        }

        return SystemDeclaration{system_name.lexeme, std::move(on_update), std::move(on_event), std::move(functions)};
    }

    OnUpdateDeclaration GParser::parse_on_update_declaration() {
        consume(TokenType::LPAR, "expected '(' after 'on_update'");
        consume(TokenType::FLOAT, "expected parameter type 'float' in 'on_update'");
        const Token& param_name = consume(TokenType::IDENTIFIER, "expected delta_time parameter name in 'on_update'");
        consume(TokenType::RPAR, "expected ')' after 'on_update' parameter");

        bool prev_hook = in_system_hook_;
        in_system_hook_ = true;
        Block body = parse_block_statement();
        in_system_hook_ = prev_hook;

        return OnUpdateDeclaration{param_name.lexeme, std::move(body)};
    }

    OnEventDeclaration GParser::parse_on_event_declaration() {
        consume(TokenType::LPAR, "expected '(' after 'on_event'");
        consume(TokenType::EVENT, "expected parameter type 'event' in 'on_event'");
        const Token& param_name = consume(TokenType::IDENTIFIER, "expected event parameter name in 'on_event'");
        consume(TokenType::RPAR, "expected ')' after 'on_event' parameter");

        bool prev_hook = in_system_hook_;
        in_system_hook_ = true;
        Block body = parse_block_statement();
        in_system_hook_ = prev_hook;

        return OnEventDeclaration{param_name.lexeme, std::move(body)};
    }

    FunctionDeclaration GParser::parse_function_declaration() {
        TypePtr return_type = parse_type();
        const Token& name = consume(TokenType::IDENTIFIER, "expected function name");
        consume(TokenType::LPAR, "expected '(' after function name");

        std::vector<Parameter> parameters;
        if (!check(TokenType::RPAR)) {
            do {
                TypePtr param_type = parse_type();
                const Token& param_name = consume(TokenType::IDENTIFIER, "expected parameter name");
                parameters.push_back(Parameter{std::move(param_type), param_name.lexeme});
            }
            while (match(TokenType::COMMA));
        }
        consume(TokenType::RPAR, "expected ')' after parameters");
        Block body = parse_block_statement();

        return FunctionDeclaration{std::move(return_type), name.lexeme, std::move(parameters), std::move(body)};
    }

    // STATEMENTS

    StatementPtr GParser::parse_statement() {
        if (check(TokenType::LBRACE)) return make_stmt<Block>(parse_block_statement());
        if (match(TokenType::IF)) return make_stmt<IfStatement>(parse_if_statement());
        if (match(TokenType::SWITCH)) return make_stmt<SwitchStatement>(parse_switch_statement());
        if (match(TokenType::FOR)) return parse_for_statement();
        if (match(TokenType::WHILE)) return make_stmt<WhileStatement>(parse_while_statement());
        if (match(TokenType::DO)) return make_stmt<DoWhileStatement>(parse_do_while_statement());
        return make_stmt<SimpleStatementPtr>(parse_simple_statement());
    }

    Block GParser::parse_block_statement() {
        consume(TokenType::LBRACE, "expected '{' before block");
        std::vector<StatementPtr> statements;
        while (!check(TokenType::RBRACE) && !is_at_end()) {
            statements.push_back(parse_statement());
        }
        consume(TokenType::RBRACE, "expected '}' after block");
        return Block{std::move(statements)};
    }

    IfStatement GParser::parse_if_statement() {
        consume(TokenType::LPAR, "expected '(' after 'if'");
        ExpressionPtr condition = parse_expression();
        consume(TokenType::RPAR, "expected ')' after if condition");

        Block then_branch = parse_block_statement();
        StatementPtr else_branch = nullptr;

        if (match(TokenType::ELSE)) {
            if (match(TokenType::IF)) {
                else_branch = make_stmt<IfStatement>(parse_if_statement());
            }
            else {
                else_branch = make_stmt<Block>(parse_block_statement());
            }
        }
        return IfStatement{std::move(condition), std::move(then_branch), std::move(else_branch)};
    }

    SwitchStatement GParser::parse_switch_statement() {
        consume(TokenType::LPAR, "expected '(' after 'switch'");
        ExpressionPtr selector = parse_expression();
        consume(TokenType::RPAR, "expected ')' after switch expression");
        consume(TokenType::LBRACE, "expected '{' before switch body");

        std::vector<SwitchCase> cases;
        while (!check(TokenType::RBRACE) && !is_at_end()) {
            ExpressionPtr label = nullptr;
            if (match(TokenType::CASE)) {
                label = parse_expression();
            }
            else if (match(TokenType::DEFAULT)) {
                label = nullptr;
            }
            else {
                throw syntax_error(peek(), "expected 'case' or 'default' in switch");
            }

            consume(TokenType::ARROW, "expected '->' after switch case label");
            StatementPtr body = check(TokenType::LBRACE) ? make_stmt<Block>(parse_block_statement()) : parse_statement();
            cases.push_back(SwitchCase{std::move(label), std::move(body)});
        }

        consume(TokenType::RBRACE, "expected '}' after switch body");
        return SwitchStatement{std::move(selector), std::move(cases)};
    }

    StatementPtr GParser::parse_for_statement() {
        const Token& for_token = previous();
        consume(TokenType::LPAR, "expected '(' after 'for'");

        // C-style for loop: ';', 'let' oder 'var'
        if (check(TokenType::SEMICOLON) || check(TokenType::LET) || check(TokenType::VAR)) {
            return make_stmt<ForCStatement>(parse_for_c_loop());
        }

        // ECS loop mit explizitem 'entity' binding: for (entity e, Comp c)
        if (check(TokenType::ENTITY) && peek_at(1).type == TokenType::IDENTIFIER && peek_at(2).type == TokenType::COMMA) {
            return make_stmt<ForEcsStatement>(parse_for_ecs_loop(for_token));
        }

        // Disambiguierung Typ + Identifier (Range-For vs. ECS-Loop)
        if (is_type_start()) {
            size_t len = type_length();
            if (peek_at(len).type == TokenType::IDENTIFIER) {
                TokenType after_id = peek_at(len + 1).type;
                if (after_id == TokenType::COLON) {
                    return make_stmt<ForRangeStatement>(parse_for_range_loop());
                }
                if (after_id == TokenType::COMMA || after_id == TokenType::RPAR) {
                    return make_stmt<ForEcsStatement>(parse_for_ecs_loop(for_token));
                }
            }
        }

        // Fallback: C-Style Loop
        return make_stmt<ForCStatement>(parse_for_c_loop());
    }

    ForCStatement GParser::parse_for_c_loop() {
        SimpleStatementPtr init = nullptr;
        if (!check(TokenType::SEMICOLON)) {
            init = parse_for_init();
        }
        consume(TokenType::SEMICOLON, "expected ';' after for-loop initializer");

        ExpressionPtr condition = nullptr;
        if (!check(TokenType::SEMICOLON)) {
            condition = parse_expression();
        }
        consume(TokenType::SEMICOLON, "expected ';' after for-loop condition");

        SimpleStatementPtr update = nullptr;
        if (!check(TokenType::RPAR)) {
            update = parse_expression_or_assignment();
        }
        consume(TokenType::RPAR, "expected ')' after for-loop clauses");

        Block body = parse_block_statement();
        return ForCStatement{std::move(init), std::move(condition), std::move(update), std::move(body)};
    }

    SimpleStatementPtr GParser::parse_for_init() {
        if (match(TokenType::LET)) return parse_variable_declaration(false);
        if (match(TokenType::VAR)) return parse_variable_declaration(true);
        return parse_expression_or_assignment();
    }

    ForRangeStatement GParser::parse_for_range_loop() {
        TypePtr element_type = parse_type();
        const Token& var_name = consume(TokenType::IDENTIFIER, "expected loop variable name");
        consume(TokenType::COLON, "expected ':' in range-for loop");
        ExpressionPtr collection = parse_expression();
        consume(TokenType::RPAR, "expected ')' after range-for collection");
        Block body = parse_block_statement();
        return ForRangeStatement{std::move(element_type), var_name.lexeme, std::move(collection), std::move(body)};
    }

    ForEcsStatement GParser::parse_for_ecs_loop(const Token& for_token) {
        if (!in_system_hook_) {
            throw syntax_error(for_token, "ECS component query loops are only allowed inside 'on_update' or 'on_event'");
        }

        std::optional<std::string> entity_var_name;
        if (match(TokenType::ENTITY)) {
            const Token& ent_id = consume(TokenType::IDENTIFIER, "expected entity variable name after 'entity'");
            entity_var_name = ent_id.lexeme;
            consume(TokenType::COMMA, "expected ',' after entity binding in ECS for-loop");
        }

        std::vector<ComponentBinding> components;
        do {
            const Token& comp_type = consume(TokenType::IDENTIFIER, "expected component type name");
            const Token& comp_var = consume(TokenType::IDENTIFIER, std::format("expected variable name for component '{}'", comp_type.lexeme));
            components.push_back(ComponentBinding{comp_type.lexeme, comp_var.lexeme});
        }
        while (match(TokenType::COMMA));

        consume(TokenType::RPAR, "expected ')' after component bindings in ECS for-loop");

        std::vector<std::string> excluded_components;
        if (match(TokenType::EXCLUDE)) {
            do {
                const Token& excluded = consume(TokenType::IDENTIFIER, "expected component type name after 'exclude'");
                excluded_components.push_back(excluded.lexeme);
            }
            while (match(TokenType::COMMA));
        }

        Block body = parse_block_statement();
        return ForEcsStatement{std::move(entity_var_name), std::move(components), std::move(excluded_components), std::move(body)};
    }

    WhileStatement GParser::parse_while_statement() {
        consume(TokenType::LPAR, "expected '(' after 'while'");
        ExpressionPtr condition = parse_expression();
        consume(TokenType::RPAR, "expected ')' after while condition");
        Block body = parse_block_statement();
        return WhileStatement{std::move(condition), std::move(body)};
    }

    DoWhileStatement GParser::parse_do_while_statement() {
        Block body = parse_block_statement();
        consume(TokenType::WHILE, "expected 'while' after do block");
        consume(TokenType::LPAR, "expected '(' after 'while'");
        ExpressionPtr condition = parse_expression();
        consume(TokenType::RPAR, "expected ')' after do-while condition");
        match(TokenType::SEMICOLON);
        return DoWhileStatement{std::move(body), std::move(condition)};
    }

    SimpleStatementPtr GParser::parse_simple_statement() {
        SimpleStatementPtr stmt;
        if (match(TokenType::LET)) {
            stmt = parse_variable_declaration(false);
        }
        else if (match(TokenType::VAR)) {
            stmt = parse_variable_declaration(true);
        }
        else if (match(TokenType::RETURN)) {
            stmt = parse_return_statement();
        }
        else if (match(TokenType::BREAK)) {
            stmt = make_simple_stmt<BreakStatement>();
        }
        else if (match(TokenType::CONTINUE)) {
            stmt = make_simple_stmt<ContinueStatement>();
        }
        else {
            stmt = parse_expression_or_assignment();
        }
        match(TokenType::SEMICOLON);
        return stmt;
    }

    SimpleStatementPtr GParser::parse_variable_declaration(bool is_mutable) {
        TypePtr type = parse_type();
        const Token& name = consume(TokenType::IDENTIFIER, "expected variable name");
        ExpressionPtr initializer = nullptr;

        if (!is_mutable) {
            consume(TokenType::EQUALS, std::format("immutable 'let' variable '{}' must be initialized", name.lexeme));
            initializer = parse_expression();
        }
        else if (match(TokenType::EQUALS)) {
            initializer = parse_expression();
        }

        return make_simple_stmt<VariableDeclaration>(is_mutable, std::move(type), name.lexeme, std::move(initializer));
    }

    SimpleStatementPtr GParser::parse_return_statement() {
        uint32_t return_line = previous().line;
        ExpressionPtr value = nullptr;

        if (!check(TokenType::SEMICOLON) && !check(TokenType::RBRACE) && !is_at_end() && peek().line == return_line) {
            value = parse_expression();
        }
        return make_simple_stmt<ReturnStatement>(std::move(value));
    }

    SimpleStatementPtr GParser::parse_expression_or_assignment() {
        ExpressionPtr expr = parse_expression();
        if (match(TokenType::EQUALS, TokenType::PLUS_EQUALS, TokenType::MINUS_EQUALS,
                  TokenType::STAR_EQUALS, TokenType::SLASH_EQUALS, TokenType::PERCENT_EQUALS)) {
            std::string op = previous().lexeme;
            ExpressionPtr value = parse_expression();
            return make_simple_stmt<Assignment>(std::move(expr), std::move(op), std::move(value));
        }
        return make_simple_stmt<ExpressionStatement>(std::move(expr));
    }

    // EXPRESSIONS

    ExpressionPtr GParser::parse_expression() {
        return parse_logical_or();
    }

    ExpressionPtr GParser::parse_logical_or() {
        ExpressionPtr expr = parse_logical_and();
        while (match(TokenType::OR_OR)) {
            std::string op = previous().lexeme;
            ExpressionPtr right = parse_logical_and();
            expr = make_expr<BinaryOp>(std::move(expr), std::move(op), std::move(right));
        }
        return expr;
    }

    ExpressionPtr GParser::parse_logical_and() {
        ExpressionPtr expr = parse_equality();
        while (match(TokenType::AND_AND)) {
            std::string op = previous().lexeme;
            ExpressionPtr right = parse_equality();
            expr = make_expr<BinaryOp>(std::move(expr), std::move(op), std::move(right));
        }
        return expr;
    }

    ExpressionPtr GParser::parse_equality() {
        ExpressionPtr expr = parse_relational();
        while (match(TokenType::EQUALS_EQUALS, TokenType::BANG_EQUALS)) {
            std::string op = previous().lexeme;
            ExpressionPtr right = parse_relational();
            expr = make_expr<BinaryOp>(std::move(expr), std::move(op), std::move(right));
        }
        return expr;
    }

    ExpressionPtr GParser::parse_relational() {
        ExpressionPtr expr = parse_additive();
        while (match(TokenType::LESS, TokenType::LESS_EQUALS, TokenType::GREATER, TokenType::GREATER_EQUALS)) {
            std::string op = previous().lexeme;
            ExpressionPtr right = parse_additive();
            expr = make_expr<BinaryOp>(std::move(expr), std::move(op), std::move(right));
        }
        return expr;
    }

    ExpressionPtr GParser::parse_additive() {
        ExpressionPtr expr = parse_multiplicative();
        while (match(TokenType::PLUS, TokenType::MINUS)) {
            std::string op = previous().lexeme;
            ExpressionPtr right = parse_multiplicative();
            expr = make_expr<BinaryOp>(std::move(expr), std::move(op), std::move(right));
        }
        return expr;
    }

    ExpressionPtr GParser::parse_multiplicative() {
        ExpressionPtr expr = parse_unary();
        while (match(TokenType::STAR, TokenType::SLASH, TokenType::PERCENT)) {
            std::string op = previous().lexeme;
            ExpressionPtr right = parse_unary();
            expr = make_expr<BinaryOp>(std::move(expr), std::move(op), std::move(right));
        }
        return expr;
    }

    ExpressionPtr GParser::parse_unary() {
        if (match(TokenType::BANG, TokenType::MINUS, TokenType::PLUS_PLUS, TokenType::MINUS_MINUS)) {
            std::string op = previous().lexeme;
            ExpressionPtr operand = parse_unary();
            return make_expr<UnaryOp>(std::move(op), std::move(operand));
        }
        return parse_postfix();
    }

    ExpressionPtr GParser::parse_postfix() {
        ExpressionPtr expr = parse_primary();
        while (true) {
            if (match(TokenType::LPAR)) {
                std::vector<ExpressionPtr> args;
                if (!check(TokenType::RPAR)) {
                    do {
                        args.push_back(parse_expression());
                    }
                    while (match(TokenType::COMMA));
                }
                consume(TokenType::RPAR, "expected ')' after arguments");
                expr = make_expr<FunctionCall>(std::move(expr), std::move(args));
            }
            else if (match(TokenType::DOT)) {
                const Token& name = consume(TokenType::IDENTIFIER, "expected member or intrinsic name after '.'");
                expr = make_expr<MemberAccess>(std::move(expr), name.lexeme);
            }
            else if (match(TokenType::LBRACKET)) {
                ExpressionPtr index = parse_expression();
                consume(TokenType::RBRACKET, "expected ']' after index");
                expr = make_expr<IndexAccess>(std::move(expr), std::move(index));
            }
            else if (match(TokenType::PLUS_PLUS, TokenType::MINUS_MINUS)) {
                expr = make_expr<PostfixOp>(previous().lexeme, std::move(expr));
            }
            else {
                break;
            }
        }
        return expr;
    }

    ExpressionPtr GParser::parse_primary() {
        if (match(TokenType::INT_LITERAL)) {
            return make_expr<IntLiteral>(std::get<int64_t>(previous().literal));
        }
        if (match(TokenType::FLOAT_LITERAL)) {
            return make_expr<FloatLiteral>(std::get<double>(previous().literal));
        }
        if (match(TokenType::BOOL_LITERAL)) {
            return make_expr<BoolLiteral>(std::get<bool>(previous().literal));
        }
        if (match(TokenType::CHAR_LITERAL)) {
            return make_expr<CharLiteral>(std::get<char>(previous().literal));
        }
        if (match(TokenType::STRING_LITERAL)) {
            return make_expr<StringLiteral>(std::get<std::string>(previous().literal));
        }
        // Math Typen-Konstruktoren wie vec3(1.0, 0.0, 0.0)
        if (match(TokenType::VEC2, TokenType::VEC3, TokenType::VEC4,
                  TokenType::MAT2, TokenType::MAT3, TokenType::MAT4, TokenType::QUAT)) {
            return make_expr<Identifier>(previous().lexeme);
        }
        if (match(TokenType::IDENTIFIER)) {
            return make_expr<Identifier>(previous().lexeme);
        }
        if (match(TokenType::LPAR)) {
            ExpressionPtr expr = parse_expression();
            consume(TokenType::RPAR, "expected ')' after expression");
            return expr;
        }

        const Token& t = peek();
        throw syntax_error(t, std::format("expected expression, found '{}'", t.lexeme));
    }

    // TYPES

    TypePtr GParser::parse_type() {
        if (match(TokenType::LIST)) {
            consume(TokenType::LESS, "expected '<' after 'list'");
            TypePtr element_type = parse_type();
            consume(TokenType::GREATER, "expected '>' after list element type");
            return Type::make_list(std::move(element_type));
        }
        if (match(TokenType::VOID, TokenType::BOOL,
                  TokenType::UINT8, TokenType::INT8, TokenType::UINT32, TokenType::INT32,
                  TokenType::UINT64, TokenType::INT64,
                  TokenType::FLOAT, TokenType::DOUBLE, TokenType::CHAR, TokenType::STRING,
                  TokenType::VEC2, TokenType::VEC3, TokenType::VEC4,
                  TokenType::MAT2, TokenType::MAT3, TokenType::MAT4, TokenType::QUAT,
                  TokenType::ENTITY, TokenType::EVENT, TokenType::IDENTIFIER)) {
            return Type::make_named(previous().lexeme);
        }

        const Token& t = peek();
        throw syntax_error(t, std::format("expected type, found '{}'", t.lexeme));
    }

    // LOOKAHEAD HELPERS

    bool GParser::is_type_start() const {
        switch (peek().type) {
            case TokenType::VOID :
            case TokenType::BOOL :
            case TokenType::UINT8 :
            case TokenType::INT8 :
            case TokenType::UINT32 :
            case TokenType::INT32 :
            case TokenType::UINT64 :
            case TokenType::INT64 :
            case TokenType::FLOAT :
            case TokenType::DOUBLE :
            case TokenType::CHAR :
            case TokenType::STRING :
            case TokenType::VEC2 :
            case TokenType::VEC3 :
            case TokenType::VEC4 :
            case TokenType::MAT2 :
            case TokenType::MAT3 :
            case TokenType::MAT4 :
            case TokenType::QUAT :
            case TokenType::ENTITY :
            case TokenType::EVENT :
            case TokenType::LIST :
            case TokenType::IDENTIFIER :
                return true;
            default :
                return false;
        }
    }

    size_t GParser::type_length() const {
        if (peek().type == TokenType::LIST && peek_at(1).type == TokenType::LESS) {
            return skip_generic_type_args(current_ + 1) - current_;
        }
        return 1;
    }

    size_t GParser::skip_generic_type_args(size_t idx) const {
        if (idx >= tokens_.size() || tokens_[idx].type != TokenType::LESS) return idx;
        int depth = 0;
        while (idx < tokens_.size()) {
            TokenType type = tokens_[idx].type;
            if (type == TokenType::LESS) {
                depth++;
            }
            else if (type == TokenType::GREATER) {
                depth--;
                idx++;
                if (depth == 0) return idx;
                continue;
            }
            idx++;
        }
        return idx;
    }

    // TOKEN NAVIGATION & ERRORS

    bool GParser::check(TokenType type) const {
        if (is_at_end()) return false;
        return peek().type == type;
    }

    const Token& GParser::previous() const {
        return tokens_[current_ - 1];
    }

    const Token& GParser::peek() const {
        return tokens_[current_];
    }

    const Token& GParser::peek_at(size_t offset) const {
        size_t index = current_ + offset;
        if (index >= tokens_.size()) {
            return tokens_.back(); // END_OF_FILE
        }
        return tokens_[index];
    }

    const Token& GParser::consume(TokenType type, std::string_view message) {
        if (check(type)) return advance();
        const Token& t = peek();
        throw syntax_error(t, std::format("{} (found '{}')", message, t.lexeme));
    }

    const Token& GParser::advance() {
        if (!is_at_end()) current_++;
        return previous();
    }

    bool GParser::is_at_end() const {
        return peek().type == TokenType::END_OF_FILE;
    }

    GlazeCompilerError GParser::syntax_error(const Token& token, std::string_view message) {
        return {std::format("syntax error: {}", message), token.line, token.column};
    }
}
