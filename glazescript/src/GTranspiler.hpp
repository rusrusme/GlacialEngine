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
    class GTranspiler {
    public:
        GTranspiler(const SymbolTable& symbols,
                    const std::unordered_map<const Expression*, SymbolPtr>& resolved,
                    const std::unordered_map<const Expression*, TypePtr>& expression_types);

        [[nodiscard]] std::string generate(const Program& program);

    private:
        void emit_preamble();
        void emit_engine_registration_helpers(const std::vector<const StructDeclaration*>& structs,
                                              const std::vector<const SystemDeclaration*>& systems);

        // Topological Sort
        [[nodiscard]] std::vector<const StructDeclaration*> topo_sort_structs(const std::vector<const StructDeclaration*>& decls);
        void collect_type_deps(const TypePtr& type,
                               const std::unordered_map<std::string, const StructDeclaration*>& known_structs,
                               std::unordered_set<std::string>& out) const;

        // Declarations
        void emit_enum(const EnumDeclaration& enum_decl);
        void emit_struct_or_component(const StructDeclaration& decl);
        void emit_system(const SystemDeclaration& sys);
        void emit_function_prototype(const FunctionDeclaration& fn);
        void emit_function(const FunctionDeclaration& fn, bool inside_system);
        [[nodiscard]] std::string format_param_list(const std::vector<Parameter>& params);

        // Statements
        [[nodiscard]] std::string emit_statements(const std::vector<StatementPtr>& statements);
        [[nodiscard]] std::string emit_statement(const StatementPtr& stmt);
        [[nodiscard]] std::string emit_statement_inner(const StatementPtr& stmt);
        [[nodiscard]] std::string emit_for_ecs(const ForEcsStatement& for_ecs);
        [[nodiscard]] std::string emit_switch(const SwitchStatement& switch_stmt);
        [[nodiscard]] std::string emit_switch_case_body(const StatementPtr& body);
        [[nodiscard]] std::string emit_for_clause(const SimpleStatementPtr& stmt);
        [[nodiscard]] std::string emit_simple_statement(const SimpleStatementPtr& stmt);
        [[nodiscard]] std::string emit_assignment_no_semicolon(const Assignment& assign);
        [[nodiscard]] std::string emit_braced(const Block& block);

        // Expressions
        [[nodiscard]] std::string emit_expr(const ExpressionPtr& expr);
        [[nodiscard]] std::string emit_member_access(const MemberAccess& ma, const Expression* expr_raw);
        [[nodiscard]] std::string emit_function_call(const FunctionCall& call, const Expression* expr_raw);
        [[nodiscard]] std::string emit_entity_intrinsic_call(const ExpressionPtr& receiver_expr,
                                                             const std::string& method,
                                                             const std::vector<ExpressionPtr>& args);
        [[nodiscard]] std::string emit_list_intrinsic_call(const ExpressionPtr& receiver_expr,
                                                           const std::string& method,
                                                           const std::vector<ExpressionPtr>& args);
        [[nodiscard]] std::string emit_call_arguments(const std::vector<ExpressionPtr>& args,
                                                      const std::vector<Parameter>& params);

        // Types & Spills
        [[nodiscard]] std::string cpp_type(const TypePtr& type) const;
        [[nodiscard]] std::string cpp_param_type(const TypePtr& type) const;
        [[nodiscard]] bool is_passed_by_reference(const TypePtr& type) const;
        [[nodiscard]] TypePtr type_of(const ExpressionPtr& expr) const;
        [[nodiscard]] TypePtr type_of(const Expression* expr) const;
        [[nodiscard]] static bool is_entity_type(const TypePtr& type);
        [[nodiscard]] bool is_enum_member_access(const ExpressionPtr& expr) const;
        [[nodiscard]] bool is_lvalue(const ExpressionPtr& expr) const;
        [[nodiscard]] std::string spill_if_needed(const ExpressionPtr& expr);
        [[nodiscard]] std::string spill(const ExpressionPtr& expr);

        // Formatting Helpers
        template<typename F>
        std::string emit_with_spills(F&& body_generator);
        [[nodiscard]] static std::string safe_name(const std::string& name);
        [[nodiscard]] static std::string format_float(double value);
        [[nodiscard]] static std::string format_char(char value);
        [[nodiscard]] static std::string format_string(const std::string& value);
        [[nodiscard]] std::string indent() const;
        void line(std::string_view text);

    private:
        static const std::unordered_set<std::string> s_cpp_keywords;
        static const std::unordered_set<std::string> s_value_primitives;
        static const std::unordered_set<std::string> s_math_types;
        static const std::unordered_set<std::string> s_builtin_math_functions;

        const SymbolTable& symbols_;
        const std::unordered_map<const Expression*, SymbolPtr>& resolved_;
        const std::unordered_map<const Expression*, TypePtr>& expression_types_;

        std::string builder_{};
        std::vector<std::string> pending_spills_{};
        int temp_counter_{0};
        int indent_{0};
        std::string program_name_{};
    };
}
