//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace glaze {
    // Helper for pattern matching
    template<class... Ts>
    struct match : Ts... {
        using Ts::operator()...;
    };

    template<class... Ts>
    match(Ts...) -> match<Ts...>;

    // FORWARD DECLARATIONS

    struct Type;
    using TypePtr = std::shared_ptr<Type>;

    struct Expression;
    using ExpressionPtr = std::unique_ptr<Expression>;

    struct SimpleStatement;
    using SimpleStatementPtr = std::unique_ptr<SimpleStatement>;

    struct Statement;
    using StatementPtr = std::unique_ptr<Statement>;

    struct Declaration;
    using DeclarationPtr = std::unique_ptr<Declaration>;

    // TYPES

    struct NamedType {
        std::string name;
        bool operator==(const NamedType&) const = default;
    };

    struct ListType {
        TypePtr element_type;
        bool operator==(const ListType& other) const;
    };

    using TypeVariant = std::variant<NamedType, ListType>;

    struct Type {
        TypeVariant node;

        template<typename T>
            requires (!std::is_same_v<std::remove_cvref_t<T>, Type> &&
                std::is_constructible_v<TypeVariant, T>)
        explicit Type(T&& val) : node(std::forward<T>(val)) {}

        Type(const Type&) = default;
        Type(Type&&) noexcept = default;
        Type& operator=(const Type&) = default;
        Type& operator=(Type&&) noexcept = default;

        static TypePtr make_named(std::string name) {
            return std::make_shared<Type>(NamedType{std::move(name)});
        }

        static TypePtr make_list(TypePtr element) {
            return std::make_shared<Type>(ListType{std::move(element)});
        }

        template<typename T>
        [[nodiscard]] bool is() const noexcept {
            return std::holds_alternative<T>(node);
        }

        template<typename T>
        [[nodiscard]] const T& as() const {
            return std::get<T>(node);
        }

        template<typename T>
        [[nodiscard]] T& as() {
            return std::get<T>(node);
        }

        bool operator==(const Type& other) const {
            return node == other.node;
        }
    };

    inline bool ListType::operator==(const ListType& other) const {
        if (!element_type && !other.element_type) return true;
        if (!element_type || !other.element_type) return false;
        return *element_type == *other.element_type;
    }

    // EXPRESSIONS

    struct IntLiteral {
        int64_t value{0};
    };

    struct FloatLiteral {
        double value{0.0};
    };

    struct BoolLiteral {
        bool value{false};
    };

    struct CharLiteral {
        char value{'\0'};
    };

    struct StringLiteral {
        std::string value;
    };

    struct Identifier {
        std::string name;
    };

    struct UnaryOp {
        std::string op;
        ExpressionPtr operand;
    };

    struct BinaryOp {
        ExpressionPtr left;
        std::string op;
        ExpressionPtr right;
    };

    struct PostfixOp {
        std::string op;
        ExpressionPtr operand;
    };

    struct FunctionCall {
        ExpressionPtr callee;
        std::vector<ExpressionPtr> arguments;
    };

    struct MemberAccess {
        ExpressionPtr object;
        std::string member;
    };

    struct IndexAccess {
        ExpressionPtr array;
        ExpressionPtr index;
    };

    using ExpressionVariant = std::variant<
        IntLiteral,
        FloatLiteral,
        BoolLiteral,
        CharLiteral,
        StringLiteral,
        Identifier,
        UnaryOp,
        BinaryOp,
        PostfixOp,
        FunctionCall,
        MemberAccess,
        IndexAccess
    >;

    struct Expression {
        ExpressionVariant node;

        template <typename T>
            requires (!std::is_same_v<std::remove_cvref_t<T>, Expression> &&
                       std::is_constructible_v<ExpressionVariant, T>)
        explicit Expression(T&& val) : node(std::forward<T>(val)) {}

        Expression(const Expression&) = delete;
        Expression& operator=(const Expression&) = delete;
        Expression(Expression&&) noexcept = default;
        Expression& operator=(Expression&&) noexcept = default;

        template<typename T>
        [[nodiscard]] bool is() const noexcept {
            return std::holds_alternative<T>(node);
        }

        template<typename T>
        [[nodiscard]] const T& as() const {
            return std::get<T>(node);
        }

        template<typename T>
        [[nodiscard]] T& as() {
            return std::get<T>(node);
        }
    };

    template<typename T, typename... Args>
    ExpressionPtr make_expr(Args&&... args) {
        return std::make_unique<Expression>(T{std::forward<Args>(args)...});
    }

    // SIMPLE STATEMENTS

    struct VariableDeclaration {
        bool is_mutable{false};
        TypePtr type;
        std::string name;
        ExpressionPtr initializer;
    };

    struct Assignment {
        ExpressionPtr target;
        std::string op; // "=", "+=", "-=", "*=", "/=", "%="
        ExpressionPtr value;
    };

    struct ReturnStatement {
        ExpressionPtr value;
    };

    struct BreakStatement {};

    struct ContinueStatement {};

    struct ExpressionStatement {
        ExpressionPtr expression;
    };

    using SimpleStatementVariant = std::variant<
        VariableDeclaration,
        Assignment,
        ReturnStatement,
        BreakStatement,
        ContinueStatement,
        ExpressionStatement
    >;

    struct SimpleStatement {
        SimpleStatementVariant node;

        template<typename T>
            requires (!std::is_same_v<std::remove_cvref_t<T>, SimpleStatement> &&
                std::is_constructible_v<SimpleStatementVariant, T>)
        explicit SimpleStatement(T&& val) : node(std::forward<T>(val)) {}

        SimpleStatement(const SimpleStatement&) = delete;
        SimpleStatement& operator=(const SimpleStatement&) = delete;
        SimpleStatement(SimpleStatement&&) noexcept = default;
        SimpleStatement& operator=(SimpleStatement&&) noexcept = default;

        template<typename T>
        [[nodiscard]] bool is() const noexcept {
            return std::holds_alternative<T>(node);
        }

        template<typename T>
        [[nodiscard]] const T& as() const {
            return std::get<T>(node);
        }

        template<typename T>
        [[nodiscard]] T& as() {
            return std::get<T>(node);
        }
    };

    template<typename T, typename... Args>
    SimpleStatementPtr make_simple_stmt(Args&&... args) {
        return std::make_unique<SimpleStatement>(T{std::forward<Args>(args)...});
    }

    // STATEMENTS

    struct Block {
        std::vector<StatementPtr> statements;
    };

    struct IfStatement {
        ExpressionPtr condition;
        Block then_branch;
        StatementPtr else_branch;
    };

    struct SwitchCase {
        ExpressionPtr label;
        StatementPtr body;
    };

    struct SwitchStatement {
        ExpressionPtr selector;
        std::vector<SwitchCase> cases;
    };

    struct ForCStatement {
        SimpleStatementPtr init;
        ExpressionPtr condition;
        SimpleStatementPtr update;
        Block body;
    };

    struct ForRangeStatement {
        TypePtr element_type;
        std::string var_name;
        ExpressionPtr collection;
        Block body;
    };

    struct ComponentBinding {
        std::string component_type;
        std::string var_name;
    };

    struct ForEcsStatement {
        std::optional<std::string> entity_var_name;
        std::vector<ComponentBinding> components;
        std::vector<std::string> excluded_components;
        Block body;
    };

    struct WhileStatement {
        ExpressionPtr condition;
        Block body;
    };

    struct DoWhileStatement {
        Block body;
        ExpressionPtr condition;
    };

    using StatementVariant = std::variant<
        Block,
        IfStatement,
        SwitchStatement,
        ForCStatement,
        ForRangeStatement,
        ForEcsStatement,
        WhileStatement,
        DoWhileStatement,
        SimpleStatementPtr
    >;

    struct Statement {
        StatementVariant node;

        template<typename T>
            requires (!std::is_same_v<std::remove_cvref_t<T>, Statement> &&
                std::is_constructible_v<StatementVariant, T>)
        explicit Statement(T&& val) : node(std::forward<T>(val)) {}

        Statement(const Statement&) = delete;
        Statement& operator=(const Statement&) = delete;
        Statement(Statement&&) noexcept = default;
        Statement& operator=(Statement&&) noexcept = default;

        template<typename T>
        [[nodiscard]] bool is() const noexcept {
            return std::holds_alternative<T>(node);
        }

        template<typename T>
        [[nodiscard]] const T& as() const {
            return std::get<T>(node);
        }

        template<typename T>
        [[nodiscard]] T& as() {
            return std::get<T>(node);
        }
    };

    template<typename T, typename... Args>
    StatementPtr make_stmt(Args&&... args) {
        return std::make_unique<Statement>(T{std::forward<Args>(args)...});
    }

    // TOP-LEVEL DECLARATIONS

    struct Field {
        TypePtr type;
        std::string name;
        ExpressionPtr initializer;
    };

    struct StructDeclaration {
        std::string name;
        std::vector<Field> fields;
        bool is_component{false};
    };

    struct EnumMember {
        std::string name;
        std::optional<int64_t> value;
    };

    struct EnumDeclaration {
        std::string name;
        std::vector<EnumMember> members;
    };

    struct Parameter {
        TypePtr type;
        std::string name;
    };

    struct FunctionDeclaration {
        TypePtr return_type;
        std::string name;
        std::vector<Parameter> parameters;
        Block body;
    };

    struct OnUpdateDeclaration {
        std::string delta_time_param;
        Block body;
    };

    struct OnEventDeclaration {
        std::string event_param;
        Block body;
    };

    struct SystemDeclaration {
        std::string name;
        std::optional<OnUpdateDeclaration> on_update;
        std::optional<OnEventDeclaration> on_event;
        std::vector<FunctionDeclaration> functions;
    };

    using DeclarationVariant = std::variant<
        StructDeclaration,
        EnumDeclaration,
        SystemDeclaration,
        FunctionDeclaration
    >;

    struct Declaration {
        DeclarationVariant node;

        template<typename T>
            requires (!std::is_same_v<std::remove_cvref_t<T>, Declaration> &&
                std::is_constructible_v<DeclarationVariant, T>)
        explicit Declaration(T&& val) : node(std::forward<T>(val)) {}

        Declaration(const Declaration&) = delete;
        Declaration& operator=(const Declaration&) = delete;
        Declaration(Declaration&&) noexcept = default;
        Declaration& operator=(Declaration&&) noexcept = default;

        template<typename T>
        [[nodiscard]] bool is() const noexcept {
            return std::holds_alternative<T>(node);
        }

        template<typename T>
        [[nodiscard]] const T& as() const {
            return std::get<T>(node);
        }

        template<typename T>
        [[nodiscard]] T& as() {
            return std::get<T>(node);
        }
    };

    template<typename T, typename... Args>
    DeclarationPtr make_decl(Args&&... args) {
        return std::make_unique<Declaration>(T{std::forward<Args>(args)...});
    }

    struct Program {
        std::string name;
        std::vector<DeclarationPtr> declarations;
    };
}
