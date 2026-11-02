//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "SymbolTable.hpp"
#include "Token.hpp"
#include "glaze/AST.hpp"

namespace glaze {
    class GChecker {
    public:
        GChecker() = default;

        void check(const Program& program);

        [[nodiscard]] const SymbolTable& symbol_table() const noexcept { return symbols_; }
        [[nodiscard]] SymbolTable& symbol_table() noexcept { return symbols_; }

        [[nodiscard]] const std::unordered_map<const Expression*, SymbolPtr>& resolved_symbols() const noexcept { return resolved_symbols_; }
        [[nodiscard]] const std::unordered_map<const Expression*, TypePtr>& expression_types() const noexcept { return expression_types_; }

    private:
        // Collect Declarations
        void collect_declarations(const std::vector<DeclarationPtr>& declarations);
        void collect_struct_or_component(const StructDeclaration& decl);
        void collect_enum(const EnumDeclaration& decl);
        void collect_system(const SystemDeclaration& decl);
        void collect_function(const FunctionDeclaration& fn, const std::string& owner_system,
                              std::unordered_map<std::string, SymbolPtr>* system_map,
                              std::vector<SymbolPtr>* system_list);
        void define_global(const SymbolPtr& symbol);

        // Type Checking
        void check_declarations(const std::vector<DeclarationPtr>& declarations);
        void check_struct_or_component(const StructDeclaration& decl);
        void check_system(const SystemDeclaration& sys);
        void check_function(const FunctionDeclaration& fn);

        // Statements
        void check_statement(const StatementPtr& stmt);
        void check_block_statements(const Block& block);
        void check_for_ecs_statement(const ForEcsStatement& for_ecs);
        void check_simple_statement(const SimpleStatementPtr& stmt);
        void check_assignment(const Assignment& assign);
        void require_mutable_lvalue(const ExpressionPtr& target);

        // Expressions
        TypePtr check_expression(const ExpressionPtr& expr);
        TypePtr check_expression_inner(const ExpressionPtr& expr);
        TypePtr check_identifier(const Identifier& id, const Expression* expr_raw);
        TypePtr check_unary(const UnaryOp& unary);
        TypePtr check_binary_operator(const std::string& op, const TypePtr& left, const TypePtr& right);
        TypePtr check_math_binary_op(const std::string& op, const TypePtr& left, const TypePtr& right);
        TypePtr check_member_access(const MemberAccess& ma, const Expression* expr_raw);
        TypePtr check_function_call(const FunctionCall& call);
        TypePtr check_entity_intrinsic(const std::string& method, const std::vector<ExpressionPtr>& args);
        TypePtr check_list_intrinsic(const std::string& method, const TypePtr& element_type, const std::vector<ExpressionPtr>& args);
        void require_mutable_if_mutating_list_method(const ExpressionPtr& receiver, const std::string& method);
        void check_call_arguments(const std::vector<ExpressionPtr>& actual_args,
                                  const std::vector<Parameter>& formal_params,
                                  const std::string& fn_name);
        TypePtr check_builtin_math_call(const std::string& name, const std::vector<ExpressionPtr>& args);
        static bool is_vector_type(const TypePtr& type);
        static bool is_matrix_type(const TypePtr& type);

        // Type Helpers
        SymbolPtr require_component_type(const std::string& name);
        void validate_type_exists(const TypePtr& type, bool allow_void);
        void require_assignable(const ExpressionPtr& expr, const TypePtr& expected, const std::string& context);
        bool assignable(const TypePtr& actual, const TypePtr& expected);
        TypePtr promote_numeric(const TypePtr& left, const TypePtr& right);
        static int numeric_rank(const TypePtr& type);
        static bool is_numeric(const TypePtr& type);
        static bool is_integer(const TypePtr& type);
        static bool is_math_type(const TypePtr& type);
        static bool is_named(const TypePtr& type, std::string_view expected);
        static bool is_vector_or_quat(const std::string& type_name);
        static bool is_valid_swizzle(const std::string& type_name, const std::string& member);
        void require_bool(const TypePtr& type, const std::string& context);
        void require_numeric(const TypePtr& type, const std::string& context);
        void require_integer(const TypePtr& type, const std::string& context);

        static std::string format_type(const TypePtr& type);
        [[nodiscard]] GlazeCompilerError error(const std::string& message) const;

    private:
        static const std::unordered_set<std::string> s_primitive_types;
        static const std::unordered_set<std::string> s_math_types;
        static const std::unordered_map<std::string, int> s_numeric_rank;
        static const std::unordered_set<std::string> s_builtin_math_functions;

        SymbolTable symbols_{};
        std::unordered_map<const Expression*, SymbolPtr> resolved_symbols_{};
        std::unordered_map<const Expression*, TypePtr> expression_types_{};

        std::string current_system_{};
        TypePtr current_return_type_{nullptr};
        int loop_depth_{0};
        int switch_depth_{0};
    };
}
