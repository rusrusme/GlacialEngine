//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "GChecker.hpp"

#include <algorithm>
#include <format>
#include <ranges>

namespace glaze {
    const std::unordered_set<std::string> GChecker::s_primitive_types = {
        "void", "bool",
        "uint8", "int8", "uint32", "int32", "uint64", "int64",
        "float", "double", "char", "string",
        "entity", "event"
    };

    const std::unordered_set<std::string> GChecker::s_math_types = {
        "vec2", "vec3", "vec4",
        "mat2", "mat3", "mat4",
        "quat"
    };

    const std::unordered_map<std::string, int> GChecker::s_numeric_rank = {
        {"int8", 0},
        {"uint8", 1},
        {"int32", 2},
        {"uint32", 3},
        {"int64", 4},
        {"uint64", 5},
        {"float", 6},
        {"double", 7}
    };

    const std::unordered_set<std::string> GChecker::s_builtin_math_functions = {
        "length", "normalize", "distance", "dot", "cross",
        "inverse", "transpose", "determinant",
        "slerp", "angleAxis",
        "abs", "sqrt", "min", "max", "clamp", "mix",
        "sin", "cos", "radians", "degrees"
    };

    void GChecker::check(const Program& program) {
        collect_declarations(program.declarations);
        check_declarations(program.declarations);
    }

    // COLLECT TOP-LEVEL DECLARATIONS

    void GChecker::collect_declarations(const std::vector<DeclarationPtr>& declarations) {
        for (const auto& decl : declarations) {
            std::visit(match{
                           [&](const StructDeclaration& s) { collect_struct_or_component(s); },
                           [&](const EnumDeclaration& e) { collect_enum(e); },
                           [&](const FunctionDeclaration& f) { collect_function(f, "", nullptr, nullptr); },
                           [&](const SystemDeclaration& sys) { collect_system(sys); }
                       }, decl->node);
        }
    }

    void GChecker::collect_struct_or_component(const StructDeclaration& decl) {
        SymbolKind kind = decl.is_component ? SymbolKind::COMPONENT : SymbolKind::STRUCT;
        TypePtr self_type = Type::make_named(decl.name);
        define_global(std::make_shared<Symbol>(decl.name, kind, self_type, true, ""));

        std::unordered_map<std::string, SymbolPtr> fields;
        std::vector<SymbolPtr> field_list;
        for (const auto& field : decl.fields) {
            auto field_sym = std::make_shared<Symbol>(field.name, SymbolKind::FIELD, field.type, false, decl.name);
            if (!fields.emplace(field.name, field_sym).second) {
                throw error(std::format("duplicate field '{}' in {}", field.name, decl.name));
            }
            field_list.push_back(field_sym);
        }
        symbols_.define_members(decl.name, std::move(fields), std::move(field_list));
    }

    void GChecker::collect_enum(const EnumDeclaration& decl) {
        TypePtr enum_type = Type::make_named(decl.name);
        define_global(std::make_shared<Symbol>(decl.name, SymbolKind::ENUM, enum_type, true, ""));

        std::unordered_map<std::string, SymbolPtr> members;
        std::vector<SymbolPtr> member_list;
        for (const auto& member : decl.members) {
            auto mem_sym = std::make_shared<Symbol>(member.name, SymbolKind::ENUM_MEMBER, enum_type, true, decl.name);
            if (!members.emplace(member.name, mem_sym).second) {
                throw error(std::format("duplicate enum member '{}' in enum '{}'", member.name, decl.name));
            }
            member_list.push_back(mem_sym);
        }
        symbols_.define_members(decl.name, std::move(members), std::move(member_list));
    }

    void GChecker::collect_system(const SystemDeclaration& decl) {
        define_global(std::make_shared<Symbol>(decl.name, SymbolKind::SYSTEM, Type::make_named(decl.name), true, ""));

        std::unordered_map<std::string, SymbolPtr> system_functions;
        std::vector<SymbolPtr> fn_list;
        for (const auto& fn : decl.functions) {
            collect_function(fn, decl.name, &system_functions, &fn_list);
        }
        symbols_.define_members(decl.name, std::move(system_functions), std::move(fn_list));
    }

    void GChecker::collect_function(const FunctionDeclaration& fn, const std::string& owner_system,
                                    std::unordered_map<std::string, SymbolPtr>* system_map,
                                    std::vector<SymbolPtr>* system_list) {
        std::unordered_set<std::string> param_names;
        for (const auto& param : fn.parameters) {
            if (!param_names.insert(param.name).second) {
                throw error(std::format("duplicate parameter '{}' in function '{}'", param.name, fn.name));
            }
        }

        auto fn_symbol = std::make_shared<Symbol>(fn.name, SymbolKind::FUNCTION, fn.return_type, true, owner_system, fn.parameters);
        if (owner_system.empty()) {
            define_global(fn_symbol);
        }
        else {
            if (!system_map->emplace(fn.name, fn_symbol).second) {
                throw error(std::format("duplicate function '{}' in system '{}'", fn.name, owner_system));
            }
            if (system_list) {
                system_list->push_back(fn_symbol);
            }
        }
    }

    void GChecker::define_global(const SymbolPtr& symbol) {
        if (!symbols_.define_global(symbol)) {
            throw error(std::format("duplicate top-level declaration '{}'", symbol->name));
        }
    }

    // RESOLVE & TYPE-CHECK BODIES

    void GChecker::check_declarations(const std::vector<DeclarationPtr>& declarations) {
        for (const auto& decl : declarations) {
            std::visit(match{
                           [&](const StructDeclaration& s) { check_struct_or_component(s); },
                           [&](const EnumDeclaration&) {},
                           [&](const FunctionDeclaration& f) { check_function(f); },
                           [&](const SystemDeclaration& sys) { check_system(sys); }
                       }, decl->node);
        }
    }

    void GChecker::check_struct_or_component(const StructDeclaration& decl) {
        for (const auto& field : decl.fields) {
            validate_type_exists(field.type, false);
            if (field.initializer) {
                require_assignable(field.initializer, field.type, std::format("default initializer of field '{}'", field.name));
            }
        }
    }

    void GChecker::check_system(const SystemDeclaration& sys) {
        std::string prev_system = current_system_;
        current_system_ = sys.name;

        if (sys.on_update.has_value()) {
            symbols_.push_scope();
            symbols_.define_local(std::make_shared<Symbol>(sys.on_update->delta_time_param, SymbolKind::VARIABLE, Type::make_named("float"), false));
            TypePtr prev_ret = current_return_type_;
            current_return_type_ = Type::make_named("void");
            check_block_statements(sys.on_update->body);
            current_return_type_ = prev_ret;
            symbols_.pop_scope();
        }

        if (sys.on_event.has_value()) {
            symbols_.push_scope();
            symbols_.define_local(std::make_shared<Symbol>(sys.on_event->event_param, SymbolKind::VARIABLE, Type::make_named("event"), false));
            TypePtr prev_ret = current_return_type_;
            current_return_type_ = Type::make_named("void");
            check_block_statements(sys.on_event->body);
            current_return_type_ = prev_ret;
            symbols_.pop_scope();
        }

        for (const auto& fn : sys.functions) {
            check_function(fn);
        }

        current_system_ = prev_system;
    }

    void GChecker::check_function(const FunctionDeclaration& fn) {
        validate_type_exists(fn.return_type, true);
        symbols_.push_scope();
        for (const auto& param : fn.parameters) {
            validate_type_exists(param.type, false);
            symbols_.define_local(std::make_shared<Symbol>(param.name, SymbolKind::VARIABLE, param.type, false));
        }

        TypePtr prev_ret = current_return_type_;
        current_return_type_ = fn.return_type;
        check_block_statements(fn.body);
        current_return_type_ = prev_ret;
        symbols_.pop_scope();
    }

    // STATEMENTS

    void GChecker::check_statement(const StatementPtr& stmt) {
        if (!stmt) return;
        std::visit(match{
                       [&](const Block& block) {
                           symbols_.push_scope();
                           check_block_statements(block);
                           symbols_.pop_scope();
                       },
                       [&](const IfStatement& if_stmt) {
                           require_bool(check_expression(if_stmt.condition), "if condition");
                           check_block_statements(if_stmt.then_branch);
                           if (if_stmt.else_branch) {
                               check_statement(if_stmt.else_branch);
                           }
                       },
                       [&](const SwitchStatement& switch_stmt) {
                           TypePtr selector_type = check_expression(switch_stmt.selector);
                           switch_depth_++;
                           for (const auto& sc : switch_stmt.cases) {
                               if (sc.label) {
                                   require_assignable(sc.label, selector_type, "switch case label");
                               }
                               check_statement(sc.body);
                           }
                           switch_depth_--;
                       },
                       [&](const ForCStatement& for_c) {
                           symbols_.push_scope();
                           if (for_c.init) check_simple_statement(for_c.init);
                           if (for_c.condition) require_bool(check_expression(for_c.condition), "for-loop condition");
                           if (for_c.update) check_simple_statement(for_c.update);
                           loop_depth_++;
                           check_block_statements(for_c.body);
                           loop_depth_--;
                           symbols_.pop_scope();
                       },
                       [&](const ForRangeStatement& for_range) {
                           validate_type_exists(for_range.element_type, false);
                           TypePtr col_type = check_expression(for_range.collection);
                           if (!col_type || !col_type->is<ListType>()) {
                               throw error(std::format("range-for loop requires a 'list<T>' collection, got {}", format_type(col_type)));
                           }
                           TypePtr elem_type = col_type->as<ListType>().element_type;
                           if (!assignable(elem_type, for_range.element_type)) {
                               throw error(std::format("range-for variable type {} does not match list element type {}",
                                                       format_type(for_range.element_type), format_type(elem_type)));
                           }
                           symbols_.push_scope();
                           symbols_.define_local(std::make_shared<Symbol>(for_range.var_name, SymbolKind::VARIABLE, for_range.element_type, false));
                           loop_depth_++;
                           check_block_statements(for_range.body);
                           loop_depth_--;
                           symbols_.pop_scope();
                       },
                       [&](const ForEcsStatement& for_ecs) {
                           check_for_ecs_statement(for_ecs);
                       },
                       [&](const WhileStatement& while_stmt) {
                           require_bool(check_expression(while_stmt.condition), "while condition");
                           loop_depth_++;
                           check_block_statements(while_stmt.body);
                           loop_depth_--;
                       },
                       [&](const DoWhileStatement& do_while) {
                           loop_depth_++;
                           check_block_statements(do_while.body);
                           loop_depth_--;
                           require_bool(check_expression(do_while.condition), "do-while condition");
                       },
                       [&](const SimpleStatementPtr& simple) {
                           check_simple_statement(simple);
                       }
                   }, stmt->node);
    }

    void GChecker::check_block_statements(const Block& block) {
        for (const auto& stmt : block.statements) {
            check_statement(stmt);
        }
    }

    void GChecker::check_for_ecs_statement(const ForEcsStatement& for_ecs) {
        symbols_.push_scope();

        if (for_ecs.entity_var_name.has_value()) {
            if (!symbols_.define_local(std::make_shared<Symbol>(*for_ecs.entity_var_name, SymbolKind::VARIABLE, Type::make_named("entity"), false))) {
                throw error(std::format("duplicate variable '{}' in ECS for-loop", *for_ecs.entity_var_name));
            }
        }

        std::unordered_set<std::string> queried_components;
        for (const auto& binding : for_ecs.components) {
            require_component_type(binding.component_type);
            if (!queried_components.insert(binding.component_type).second) {
                throw error(std::format("duplicate component '{}' in ECS query", binding.component_type));
            }
            auto comp_var = std::make_shared<Symbol>(binding.var_name, SymbolKind::VARIABLE, Type::make_named(binding.component_type), false);
            if (!symbols_.define_local(comp_var)) {
                throw error(std::format("duplicate variable '{}' in ECS for-loop", binding.var_name));
            }
        }

        for (const auto& excluded : for_ecs.excluded_components) {
            require_component_type(excluded);
            if (queried_components.contains(excluded)) {
                throw error(std::format("component '{}' cannot be both queried and excluded", excluded));
            }
        }

        loop_depth_++;
        check_block_statements(for_ecs.body);
        loop_depth_--;
        symbols_.pop_scope();
    }

    void GChecker::check_simple_statement(const SimpleStatementPtr& stmt) {
        if (!stmt) return;
        std::visit(match{
                       [&](const VariableDeclaration& var_decl) {
                           validate_type_exists(var_decl.type, false);
                           if (var_decl.initializer) {
                               require_assignable(var_decl.initializer, var_decl.type, std::format("initializer of '{}'", var_decl.name));
                           }
                           bool is_constant = !var_decl.is_mutable;
                           auto sym = std::make_shared<Symbol>(var_decl.name, SymbolKind::VARIABLE, var_decl.type, is_constant);
                           if (!symbols_.define_local(sym)) {
                               throw error(std::format("duplicate local variable '{}'", var_decl.name));
                           }
                       },
                       [&](const Assignment& assign) {
                           check_assignment(assign);
                       },
                       [&](const ReturnStatement& ret) {
                           if (!ret.value) {
                               if (!is_named(current_return_type_, "void")) {
                                   throw error(std::format("missing return value in function returning {}", format_type(current_return_type_)));
                               }
                           }
                           else {
                               if (is_named(current_return_type_, "void")) {
                                   throw error("cannot return a value from a void function");
                               }
                               require_assignable(ret.value, current_return_type_, "return statement");
                           }
                       },
                       [&](const BreakStatement&) {
                           if (loop_depth_ == 0 && switch_depth_ == 0) {
                               throw error("'break' is only allowed inside a loop or switch");
                           }
                       },
                       [&](const ContinueStatement&) {
                           if (loop_depth_ == 0) {
                               throw error("'continue' is only allowed inside a loop");
                           }
                       },
                       [&](const ExpressionStatement& expr_stmt) {
                           check_expression(expr_stmt.expression);
                       }
                   }, stmt->node);
    }

    void GChecker::check_assignment(const Assignment& assign) {
        TypePtr target_type = check_expression(assign.target);
        require_mutable_lvalue(assign.target);

        if (assign.op == "=") {
            require_assignable(assign.value, target_type, "assignment");
        }
        else {
            TypePtr val_type = check_expression(assign.value);
            std::string base_op = assign.op.substr(0, 1); // "+", "-", "*", "/", "%"
            TypePtr result_type = check_binary_operator(base_op, target_type, val_type);
            if (!assignable(result_type, target_type)) {
                throw error(std::format("cannot apply '{}' to {} and {}",
                                        assign.op, format_type(target_type), format_type(val_type)));
            }
        }
    }

    void GChecker::require_mutable_lvalue(const ExpressionPtr& target) {
        if (!target) throw error("invalid assignment target");
        std::visit(match{
                       [&](const Identifier& id) {
                           auto it = resolved_symbols_.find(target.get());
                           SymbolPtr sym = (it != resolved_symbols_.end()) ? it->second : nullptr;
                           if (!sym || sym->kind != SymbolKind::VARIABLE) {
                               throw error(std::format("invalid assignment target '{}'", id.name));
                           }
                           if (sym->constant) {
                               throw error(std::format("cannot assign to immutable 'let' variable '{}'", id.name));
                           }
                       },
                       [&](const MemberAccess& ma) {
                           auto it = resolved_symbols_.find(target.get());
                           SymbolPtr sym = (it != resolved_symbols_.end()) ? it->second : nullptr;
                           if (sym && sym->constant) {
                               throw error(std::format("cannot assign to read-only property '{}'", ma.member));
                           }
                           require_mutable_lvalue(ma.object);
                       },
                       [&](const IndexAccess& ia) {
                           require_mutable_lvalue(ia.array);
                       },
                       [&](const auto&) {
                           throw error("invalid assignment target");
                       }
                   }, target->node);
    }

    // EXPRESSIONS

    TypePtr GChecker::check_expression(const ExpressionPtr& expr) {
        if (!expr) return nullptr;
        TypePtr type = check_expression_inner(expr);
        expression_types_[expr.get()] = type;
        return type;
    }

    TypePtr GChecker::check_expression_inner(const ExpressionPtr& expr) {
        return std::visit(match{
                              [&](const IntLiteral&) -> TypePtr { return Type::make_named("int32"); },
                              [&](const FloatLiteral&) -> TypePtr { return Type::make_named("float"); },
                              [&](const BoolLiteral&) -> TypePtr { return Type::make_named("bool"); },
                              [&](const CharLiteral&) -> TypePtr { return Type::make_named("char"); },
                              [&](const StringLiteral&) -> TypePtr { return Type::make_named("string"); },
                              [&](const Identifier& id) -> TypePtr { return check_identifier(id, expr.get()); },
                              [&](const UnaryOp& unary) -> TypePtr { return check_unary(unary); },
                              [&](const PostfixOp& postfix) -> TypePtr {
                                  TypePtr operand_type = check_expression(postfix.operand);
                                  require_numeric(operand_type, std::format("postfix '{}'", postfix.op));
                                  require_mutable_lvalue(postfix.operand);
                                  return operand_type;
                              },
                              [&](const BinaryOp& binary) -> TypePtr {
                                  TypePtr left = check_expression(binary.left);
                                  TypePtr right = check_expression(binary.right);
                                  return check_binary_operator(binary.op, left, right);
                              },
                              [&](const MemberAccess& ma) -> TypePtr { return check_member_access(ma, expr.get()); },
                              [&](const IndexAccess& ia) -> TypePtr {
                                  TypePtr col_type = check_expression(ia.array);
                                  if (!col_type || !col_type->is<ListType>()) {
                                      throw error(std::format("indexing requires a 'list<T>', got {}", format_type(col_type)));
                                  }
                                  TypePtr idx_type = check_expression(ia.index);
                                  require_integer(idx_type, "list index");
                                  return col_type->as<ListType>().element_type;
                              },
                              [&](const FunctionCall& call) -> TypePtr { return check_function_call(call); }
                          }, expr->node);
    }

    TypePtr GChecker::check_identifier(const Identifier& id, const Expression* expr_raw) {
        SymbolPtr sym = symbols_.lookup(id.name);
        if (!sym) {
            throw error(std::format("undefined identifier '{}'", id.name));
        }
        if (sym->kind != SymbolKind::VARIABLE) {
            throw error(std::format("'{}' is not a variable", id.name));
        }
        resolved_symbols_[expr_raw] = sym;
        return sym->type;
    }

    TypePtr GChecker::check_unary(const UnaryOp& unary) {
        TypePtr operand = check_expression(unary.operand);
        if (unary.op == "!") {
            require_bool(operand, "unary '!'");
            return Type::make_named("bool");
        }
        if (unary.op == "-") {
            if (!is_numeric(operand) && !is_math_type(operand)) {
                throw error(std::format("unary '-' requires a numeric or math type, got {}", format_type(operand)));
            }
            return operand;
        }
        if (unary.op == "++" || unary.op == "--") {
            require_numeric(operand, std::format("prefix '{}'", unary.op));
            require_mutable_lvalue(unary.operand);
            return operand;
        }
        throw error(std::format("unknown unary operator '{}'", unary.op));
    }

    TypePtr GChecker::check_binary_operator(const std::string& op, const TypePtr& left, const TypePtr& right) {
        if (op == "&&" || op == "||") {
            require_bool(left, std::format("logical '{}'", op));
            require_bool(right, std::format("logical '{}'", op));
            return Type::make_named("bool");
        }
        if (op == "==" || op == "!=") {
            if (*left != *right && !(is_numeric(left) && is_numeric(right))) {
                throw error(std::format("cannot compare {} and {}", format_type(left), format_type(right)));
            }
            return Type::make_named("bool");
        }
        if (op == "<" || op == ">" || op == "<=" || op == ">=") {
            require_numeric(left, std::format("comparison '{}'", op));
            require_numeric(right, std::format("comparison '{}'", op));
            return Type::make_named("bool");
        }
        if (op == "+" || op == "-" || op == "*" || op == "/") {
            if (op == "+" && is_named(left, "string") && is_named(right, "string")) {
                return Type::make_named("string");
            }
            if (is_math_type(left) || is_math_type(right)) {
                return check_math_binary_op(op, left, right);
            }
            require_numeric(left, std::format("'{}'", op));
            require_numeric(right, std::format("'{}'", op));
            return promote_numeric(left, right);
        }
        if (op == "%") {
            require_integer(left, "'%'");
            require_integer(right, "'%'");
            return promote_numeric(left, right);
        }
        throw error(std::format("unknown binary operator '{}'", op));
    }

    TypePtr GChecker::check_math_binary_op(const std::string& op, const TypePtr& left, const TypePtr& right) {
        if (*left == *right) {
            return left; // e.g. vec3 + vec3, mat4 * mat4
        }
        if (op == "*" || op == "/") {
            if (is_math_type(left) && is_numeric(right)) return left;
            if (is_numeric(left) && is_math_type(right) && op == "*") return right;
            if (is_named(left, "mat2") && is_named(right, "vec2") && op == "*") return right;
            if (is_named(left, "mat3") && is_named(right, "vec3") && op == "*") return right;
            if (is_named(left, "mat4") && is_named(right, "vec4") && op == "*") return right;
            if (is_named(left, "quat") && is_named(right, "vec3") && op == "*") return right;
        }
        throw error(std::format("invalid math operation: {} {} {}", format_type(left), op, format_type(right)));
    }

    // MEMBER ACCESS & BUILT-IN PROPERTIES

    TypePtr GChecker::check_member_access(const MemberAccess& ma, const Expression* expr_raw) {
        // 1. Enum Member Access: BodyType.DYNAMIC
        if (ma.object->is<Identifier>()) {
            const std::string& name = ma.object->as<Identifier>().name;
            SymbolPtr global_sym = symbols_.lookup_global(name);
            if (global_sym && global_sym->kind == SymbolKind::ENUM) {
                SymbolPtr enum_member = symbols_.lookup_member(name, ma.member);
                if (!enum_member) {
                    throw error(std::format("unknown enum value '{}' in enum '{}'", ma.member, name));
                }
                resolved_symbols_[expr_raw] = enum_member;
                return enum_member->type;
            }
        }

        TypePtr obj_type = check_expression(ma.object);

        // 2. Built-in 'list<T>' properties: list.size
        if (obj_type && obj_type->is<ListType>()) {
            if (ma.member == "size") {
                auto size_sym = std::make_shared<Symbol>("size", SymbolKind::FIELD, Type::make_named("uint32"), true, "list");
                resolved_symbols_[expr_raw] = size_sym;
                return size_sym->type;
            }
            throw error(std::format("unknown property '{}' on list (did you mean to call a method?)", ma.member));
        }

        if (obj_type && obj_type->is<NamedType>()) {
            const std::string& type_name = obj_type->as<NamedType>().name;

            // 3. Built-in 'entity' properties: name, id, active
            if (type_name == "entity") {
                if (ma.member == "id") {
                    auto s = std::make_shared<Symbol>("id", SymbolKind::FIELD, Type::make_named("uint32"), true, "entity");
                    resolved_symbols_[expr_raw] = s;
                    return s->type;
                }
                if (ma.member == "name") {
                    auto s = std::make_shared<Symbol>("name", SymbolKind::FIELD, Type::make_named("string"), false, "entity");
                    resolved_symbols_[expr_raw] = s;
                    return s->type;
                }
                if (ma.member == "active") {
                    auto s = std::make_shared<Symbol>("active", SymbolKind::FIELD, Type::make_named("bool"), false, "entity");
                    resolved_symbols_[expr_raw] = s;
                    return s->type;
                }
                throw error(std::format("unknown property '{}' on entity", ma.member));
            }

            // 4. Math vector component access (.x, .y, .z, .w / .r, .g, .b, .a)
            if (is_vector_or_quat(type_name) && is_valid_swizzle(type_name, ma.member)) {
                size_t len = ma.member.length();
                TypePtr res_type = (len == 1) ? Type::make_named("float") : Type::make_named(std::format("vec{}", len));
                bool is_read_only = len > 1;
                auto s = std::make_shared<Symbol>(ma.member, SymbolKind::FIELD, res_type, is_read_only, type_name);
                resolved_symbols_[expr_raw] = s;
                return res_type;
            }

            // 5. Struct / Component field access
            SymbolPtr field_sym = symbols_.lookup_member(type_name, ma.member);
            if (field_sym && field_sym->kind == SymbolKind::FIELD) {
                resolved_symbols_[expr_raw] = field_sym;
                return field_sym->type;
            }
        }

        throw error(std::format("unknown member '{}' on type {}", ma.member, format_type(obj_type)));
    }

    // FUNCTION, CONSTRUCTOR & INTRINSIC CALLS

    TypePtr GChecker::check_function_call(const FunctionCall& call) {
        // Direct Identifier Call
        if (call.callee->is<Identifier>()) {
            const std::string& name = call.callee->as<Identifier>().name;

            if (s_math_types.contains(name)) {
                for (const auto& arg : call.arguments) {
                    TypePtr arg_type = check_expression(arg);
                    if (!is_numeric(arg_type) && !is_math_type(arg_type)) {
                        throw error(std::format("math constructor '{}' requires numeric or math arguments, got {}",
                                                name, format_type(arg_type)));
                    }
                }
                return Type::make_named(name);
            }

            if (s_builtin_math_functions.contains(name)) {
                return check_builtin_math_call(name, call.arguments);
            }

            SymbolPtr fn_sym = nullptr;
            if (!current_system_.empty()) {
                fn_sym = symbols_.lookup_member(current_system_, name);
            }
            if (!fn_sym) {
                fn_sym = symbols_.lookup_global(name);
            }
            if (!fn_sym) {
                throw error(std::format("undefined function or type '{}'", name));
            }

            resolved_symbols_[call.callee.get()] = fn_sym;

            if (fn_sym->kind == SymbolKind::FUNCTION) {
                check_call_arguments(call.arguments, fn_sym->parameters, name);
                return fn_sym->type;
            }
            if (fn_sym->kind == SymbolKind::STRUCT || fn_sym->kind == SymbolKind::COMPONENT) {
                const auto* field_list = symbols_.lookup_member_list(name);
                if (!call.arguments.empty() && field_list) {
                    if (call.arguments.size() > field_list->size()) {
                        throw error(std::format("too many arguments for '{}' constructor", name));
                    }
                    for (size_t i = 0; i < call.arguments.size(); ++i) {
                        require_assignable(call.arguments[i], (*field_list)[i]->type,
                                           std::format("field '{}' of '{}'", (*field_list)[i]->name, name));
                    }
                }
                return fn_sym->type;
            }
            throw error(std::format("'{}' cannot be called as a function", name));
        }

        // Member / Intrinsic Call
        if (call.callee->is<MemberAccess>()) {
            const auto& ma = call.callee->as<MemberAccess>();

            // Static system helper call: TrafficSystem.helper(...)
            if (ma.object->is<Identifier>()) {
                const std::string& sys_name = ma.object->as<Identifier>().name;
                SymbolPtr sys_sym = symbols_.lookup_global(sys_name);
                if (sys_sym && sys_sym->kind == SymbolKind::SYSTEM) {
                    SymbolPtr helper_sym = symbols_.lookup_member(sys_name, ma.member);
                    if (!helper_sym || helper_sym->kind != SymbolKind::FUNCTION) {
                        throw error(std::format("unknown function '{}' in system '{}'", ma.member, sys_name));
                    }
                    resolved_symbols_[call.callee.get()] = helper_sym;
                    check_call_arguments(call.arguments, helper_sym->parameters, std::format("{}.{}", sys_name, ma.member));
                    return helper_sym->type;
                }
            }

            TypePtr receiver_type = check_expression(ma.object);
            if (is_named(receiver_type, "entity")) {
                return check_entity_intrinsic(ma.member, call.arguments);
            }
            if (receiver_type && receiver_type->is<ListType>()) {
                require_mutable_if_mutating_list_method(ma.object, ma.member);
                return check_list_intrinsic(ma.member, receiver_type->as<ListType>().element_type, call.arguments);
            }

            throw error(std::format("type {} has no method '{}'", format_type(receiver_type), ma.member));
        }

        throw error("expression is not callable");
    }

    TypePtr GChecker::check_entity_intrinsic(const std::string& method, const std::vector<ExpressionPtr>& args) {
        if (args.empty() || !args.front()->is<Identifier>()) {
            throw error(std::format("entity.{}(...) requires a component type identifier as its first argument", method));
        }
        const std::string& comp_name = args.front()->as<Identifier>().name;
        SymbolPtr comp_sym = require_component_type(comp_name);
        resolved_symbols_[args.front().get()] = comp_sym;
        expression_types_[args.front().get()] = comp_sym->type;

        if (method == "get") {
            if (args.size() != 1) throw error("entity.get expects exactly 1 component type argument");
            return comp_sym->type;
        }
        if (method == "has") {
            if (args.size() != 1) throw error("entity.has expects exactly 1 component type argument");
            return Type::make_named("bool");
        }
        if (method == "remove") {
            if (args.size() != 1) throw error("entity.remove expects exactly 1 component type argument");
            return Type::make_named("void");
        }
        if (method == "add") {
            for (size_t i = 1; i < args.size(); ++i) {
                check_expression(args[i]);
            }
            return comp_sym->type;
        }
        throw error(std::format("unknown entity intrinsic '{}' (expected get, add, has, or remove)", method));
    }

    TypePtr GChecker::check_list_intrinsic(const std::string& method, const TypePtr& element_type, const std::vector<ExpressionPtr>& args) {
        if (method == "add") {
            if (args.size() != 1) throw error("list.add expects 1 argument");
            require_assignable(args.front(), element_type, "list.add");
            return Type::make_named("void");
        }
        if (method == "get") {
            if (args.size() != 1) throw error("list.get expects 1 index argument");
            require_integer(check_expression(args.front()), "list.get index");
            return element_type;
        }
        if (method == "remove_at") {
            if (args.size() != 1) throw error("list.remove_at expects 1 index argument");
            require_integer(check_expression(args.front()), "list.remove_at index");
            return Type::make_named("void");
        }
        if (method == "remove") {
            if (args.size() != 1) throw error("list.remove expects 1 element argument");
            require_assignable(args.front(), element_type, "list.remove");
            return Type::make_named("void");
        }
        if (method == "has") {
            if (args.size() != 1) throw error("list.has expects 1 element argument");
            require_assignable(args.front(), element_type, "list.has");
            return Type::make_named("bool");
        }
        if (method == "clear") {
            if (!args.empty()) throw error("list.clear takes no arguments");
            return Type::make_named("void");
        }
        throw error(std::format("unknown list method '{}'", method));
    }

    void GChecker::require_mutable_if_mutating_list_method(const ExpressionPtr& receiver, const std::string& method) {
        if (method == "add" || method == "remove" || method == "remove_at" || method == "clear") {
            require_mutable_lvalue(receiver);
        }
    }

    void GChecker::check_call_arguments(const std::vector<ExpressionPtr>& actual_args,
                                        const std::vector<Parameter>& formal_params,
                                        const std::string& fn_name) {
        if (actual_args.size() != formal_params.size()) {
            throw error(std::format("function '{}' expects {} arguments, got {}",
                                    fn_name, formal_params.size(), actual_args.size()));
        }
        for (size_t i = 0; i < formal_params.size(); ++i) {
            require_assignable(actual_args[i], formal_params[i].type,
                               std::format("argument {} of '{}'", i + 1, fn_name));
        }
    }

    TypePtr GChecker::check_builtin_math_call(const std::string& name, const std::vector<ExpressionPtr>& args) {
        std::vector<TypePtr> arg_types;
        arg_types.reserve(args.size());
        for (const auto& arg : args) {
            arg_types.push_back(check_expression(arg));
        }

        auto require_arity = [&](size_t expected) {
            if (arg_types.size() != expected) {
                throw error(std::format("built-in function '{}' expects {} argument(s), got {}",
                                        name, expected, arg_types.size()));
            }
        };

        // 1. Vektor-Funktionen
        if (name == "length") {
            require_arity(1);
            if (!is_vector_type(arg_types[0])) {
                throw error(std::format("'length' requires a vector type ('vec2', 'vec3', 'vec4'), got {}", format_type(arg_types[0])));
            }
            return Type::make_named("float");
        }
        if (name == "normalize") {
            require_arity(1);
            if (!is_vector_type(arg_types[0]) && !is_named(arg_types[0], "quat")) {
                throw error(std::format("'normalize' requires a vector or quat type, got {}", format_type(arg_types[0])));
            }
            return arg_types[0];
        }
        if (name == "distance" || name == "dot") {
            require_arity(2);
            if (!is_vector_type(arg_types[0]) || *arg_types[0] != *arg_types[1]) {
                throw error(std::format("'{}' requires two vectors of the same type, got {} and {}",
                                        name, format_type(arg_types[0]), format_type(arg_types[1])));
            }
            return Type::make_named("float");
        }
        if (name == "cross") {
            require_arity(2);
            if (!is_named(arg_types[0], "vec3") || !is_named(arg_types[1], "vec3")) {
                throw error(std::format("'cross' requires two 'vec3' arguments, got {} and {}",
                                        format_type(arg_types[0]), format_type(arg_types[1])));
            }
            return Type::make_named("vec3");
        }

        // 2. Matrix-Funktionen
        if (name == "inverse" || name == "transpose") {
            require_arity(1);
            if (!is_matrix_type(arg_types[0])) {
                throw error(std::format("'{}' requires a matrix type ('mat2', 'mat3', 'mat4'), got {}", name, format_type(arg_types[0])));
            }
            return arg_types[0];
        }
        if (name == "determinant") {
            require_arity(1);
            if (!is_matrix_type(arg_types[0])) {
                throw error(std::format("'determinant' requires a matrix type ('mat2', 'mat3', 'mat4'), got {}", format_type(arg_types[0])));
            }
            return Type::make_named("float");
        }

        // 3. Quaternion-Funktionen
        if (name == "slerp") {
            require_arity(3);
            if (!is_named(arg_types[0], "quat") || !is_named(arg_types[1], "quat") || !is_numeric(arg_types[2])) {
                throw error(std::format("'slerp' expects (quat, quat, float), got ({}, {}, {})",
                                        format_type(arg_types[0]), format_type(arg_types[1]), format_type(arg_types[2])));
            }
            return Type::make_named("quat");
        }
        if (name == "angleAxis") {
            require_arity(2);
            if (!is_numeric(arg_types[0]) || !is_named(arg_types[1], "vec3")) {
                throw error(std::format("'angleAxis' expects (float, vec3), got ({}, {})",
                                        format_type(arg_types[0]), format_type(arg_types[1])));
            }
            return Type::make_named("quat");
        }

        // 4. Interpolation & Bounds
        if (name == "abs") {
            require_arity(1);
            if (!is_numeric(arg_types[0]) && !is_vector_type(arg_types[0])) {
                throw error(std::format("'abs' requires a numeric or vector type, got {}", format_type(arg_types[0])));
            }
            return arg_types[0];
        }
        if (name == "min" || name == "max") {
            require_arity(2);
            if (is_vector_type(arg_types[0]) && *arg_types[0] == *arg_types[1]) {
                return arg_types[0];
            }
            if (is_numeric(arg_types[0]) && is_numeric(arg_types[1])) {
                return promote_numeric(arg_types[0], arg_types[1]);
            }
            throw error(std::format("'{}' requires matching numeric or vector types, got {} and {}",
                                    name, format_type(arg_types[0]), format_type(arg_types[1])));
        }
        if (name == "clamp") {
            require_arity(3);
            if (is_vector_type(arg_types[0]) && *arg_types[0] == *arg_types[1] && *arg_types[0] == *arg_types[2]) {
                return arg_types[0];
            }
            if (is_numeric(arg_types[0]) && is_numeric(arg_types[1]) && is_numeric(arg_types[2])) {
                return promote_numeric(arg_types[0], promote_numeric(arg_types[1], arg_types[2]));
            }
            throw error(std::format("'clamp' requires matching numeric or vector arguments, got ({}, {}, {})",
                                    format_type(arg_types[0]), format_type(arg_types[1]), format_type(arg_types[2])));
        }
        if (name == "mix") {
            require_arity(3);
            if (!is_numeric(arg_types[2])) {
                throw error(std::format("third argument of 'mix' must be a float factor, got {}", format_type(arg_types[2])));
            }
            if (is_vector_type(arg_types[0]) && *arg_types[0] == *arg_types[1]) {
                return arg_types[0];
            }
            if (is_numeric(arg_types[0]) && is_numeric(arg_types[1])) {
                return Type::make_named("float");
            }
            throw error(std::format("'mix' requires matching numeric or vector types for first two arguments, got {} and {}",
                                    format_type(arg_types[0]), format_type(arg_types[1])));
        }

        // 5. Trigonometrie & Wurzeln
        if (name == "sin" || name == "cos" || name == "radians" || name == "degrees" || name == "sqrt") {
            require_arity(1);
            if (is_vector_type(arg_types[0])) {
                return arg_types[0];
            }
            if (is_numeric(arg_types[0])) {
                return is_named(arg_types[0], "double") ? Type::make_named("double") : Type::make_named("float");
            }
            throw error(std::format("'{}' requires a numeric or vector argument, got {}", name, format_type(arg_types[0])));
        }

        throw error(std::format("unknown built-in math function '{}'", name));
    }

    bool GChecker::is_vector_type(const TypePtr& type) {
        if (!type || !type->is<NamedType>()) return false;
        const auto& n = type->as<NamedType>().name;
        return n == "vec2" || n == "vec3" || n == "vec4";
    }

    bool GChecker::is_matrix_type(const TypePtr& type) {
        if (!type || !type->is<NamedType>()) return false;
        const auto& n = type->as<NamedType>().name;
        return n == "mat2" || n == "mat3" || n == "mat4";
    }

    // TYPE HELPERS

    SymbolPtr GChecker::require_component_type(const std::string& name) {
        SymbolPtr sym = symbols_.lookup_global(name);
        if (!sym) {
            throw error(std::format("unknown component type '{}'", name));
        }
        if (sym->kind != SymbolKind::COMPONENT) {
            throw error(std::format("'{}' is not a component", name));
        }
        return sym;
    }

    void GChecker::validate_type_exists(const TypePtr& type, bool allow_void) {
        if (!type) return;
        if (type->is<ListType>()) {
            validate_type_exists(type->as<ListType>().element_type, false);
            return;
        }
        if (type->is<NamedType>()) {
            const std::string& name = type->as<NamedType>().name;
            if (name == "void") {
                if (!allow_void) throw error("'void' cannot be used as a variable or field type");
                return;
            }
            if (s_primitive_types.contains(name) || s_math_types.contains(name)) {
                return;
            }
            SymbolPtr sym = symbols_.lookup_global(name);
            if (!sym || (sym->kind != SymbolKind::STRUCT &&
                sym->kind != SymbolKind::COMPONENT &&
                sym->kind != SymbolKind::ENUM)) {
                throw error(std::format("unknown type '{}'", name));
            }
        }
    }

    void GChecker::require_assignable(const ExpressionPtr& expr, const TypePtr& expected, const std::string& context) {
        TypePtr actual = check_expression(expr);
        if (!assignable(actual, expected)) {
            throw error(std::format("type mismatch in {}: cannot convert {} to {}",
                                    context, format_type(actual), format_type(expected)));
        }
    }

    bool GChecker::assignable(const TypePtr& actual, const TypePtr& expected) {
        if (!actual || !expected) return false;
        if (*actual == *expected) return true;
        if (is_numeric(actual) && is_numeric(expected)) {
            int rank_a = numeric_rank(actual);
            int rank_e = numeric_rank(expected);
            return rank_a <= rank_e || is_named(actual, "int32");
        }
        return false;
    }

    TypePtr GChecker::promote_numeric(const TypePtr& left, const TypePtr& right) {
        int rank = std::max(numeric_rank(left), numeric_rank(right));
        switch (rank) {
            case 0 : return Type::make_named("int8");
            case 1 : return Type::make_named("uint8");
            case 2 : return Type::make_named("int32");
            case 3 : return Type::make_named("uint32");
            case 4 : return Type::make_named("int64");
            case 5 : return Type::make_named("uint64");
            case 6 : return Type::make_named("float");
            default : return Type::make_named("double");
        }
    }

    int GChecker::numeric_rank(const TypePtr& type) {
        if (!type || !type->is<NamedType>()) return -1;
        auto it = s_numeric_rank.find(type->as<NamedType>().name);
        return (it != s_numeric_rank.end()) ? it->second : -1;
    }

    bool GChecker::is_numeric(const TypePtr& type) {
        return numeric_rank(type) >= 0;
    }

    bool GChecker::is_integer(const TypePtr& type) {
        int rank = numeric_rank(type);
        return rank >= 0 && rank <= 5;
    }

    bool GChecker::is_math_type(const TypePtr& type) {
        if (!type || !type->is<NamedType>()) return false;
        return s_math_types.contains(type->as<NamedType>().name);
    }

    bool GChecker::is_named(const TypePtr& type, std::string_view expected) {
        if (!type || !type->is<NamedType>()) return false;
        return type->as<NamedType>().name == expected;
    }

    bool GChecker::is_vector_or_quat(const std::string& type_name) {
        return type_name == "vec2" || type_name == "vec3" || type_name == "vec4" || type_name == "quat";
    }

    bool GChecker::is_valid_swizzle(const std::string& type_name, const std::string& member) {
        if (member.empty() || member.length() > 4) return false;
        std::string_view allowed;
        if (type_name == "vec2") allowed = "xyrg";
        else if (type_name == "vec3") allowed = "xyzrgb";
        else allowed = "xyzwrgba";

        return std::ranges::all_of(member, [allowed](char c) {
            return allowed.find(c) != std::string_view::npos;
        });
    }

    void GChecker::require_bool(const TypePtr& type, const std::string& context) {
        if (!is_named(type, "bool")) {
            throw error(std::format("{} requires 'bool', got {}", context, format_type(type)));
        }
    }

    void GChecker::require_numeric(const TypePtr& type, const std::string& context) {
        if (!is_numeric(type)) {
            throw error(std::format("{} requires a numeric type, got {}", context, format_type(type)));
        }
    }

    void GChecker::require_integer(const TypePtr& type, const std::string& context) {
        if (!is_integer(type)) {
            throw error(std::format("{} requires an integer type, got {}", context, format_type(type)));
        }
    }

    std::string GChecker::format_type(const TypePtr& type) {
        if (!type) return "unknown";
        if (type->is<NamedType>()) {
            return type->as<NamedType>().name;
        }
        if (type->is<ListType>()) {
            return std::format("list<{}>", format_type(type->as<ListType>().element_type));
        }
        return "unknown";
    }

    GlazeCompilerError GChecker::error(const std::string& message) const {
        return {std::format("semantic error: {}", message), 0, 0};
    }
}
