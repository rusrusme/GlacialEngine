//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "glaze/AST.hpp"

namespace glaze {
    enum class SymbolKind {
        VARIABLE,
        FUNCTION,
        STRUCT,
        COMPONENT,
        ENUM,
        ENUM_MEMBER,
        SYSTEM,
        FIELD
    };

    struct Symbol {
        std::string name;
        SymbolKind kind;
        TypePtr type{};
        bool constant{false};
        std::string owner{};
        std::vector<Parameter> parameters{};

        Symbol() = default;

        Symbol(std::string name, SymbolKind kind, TypePtr type, bool constant = false,
               std::string owner = "", std::vector<Parameter> parameters = {})
            : name(std::move(name)), kind(kind), type(std::move(type)),
              constant(constant), owner(std::move(owner)), parameters(std::move(parameters)) {}
    };

    using SymbolPtr = std::shared_ptr<Symbol>;

    class SymbolTable {
    public:
        SymbolTable();

        bool define_global(const SymbolPtr& symbol);
        [[nodiscard]] SymbolPtr lookup_global(const std::string& name) const;

        void define_members(const std::string& owner_name,
                            std::unordered_map<std::string, SymbolPtr> members,
                            std::vector<SymbolPtr> member_list = {});
        [[nodiscard]] const std::unordered_map<std::string, SymbolPtr>* lookup_members(const std::string& owner_name) const;
        [[nodiscard]] const std::vector<SymbolPtr>* lookup_member_list(const std::string& owner_name) const;
        [[nodiscard]] SymbolPtr lookup_member(const std::string& owner_name, const std::string& member_name) const;

        void push_scope();
        void pop_scope();
        bool define_local(const SymbolPtr& symbol);
        [[nodiscard]] SymbolPtr lookup(const std::string& name) const;

    private:
        void register_engine_builtins();
        void register_builtin_component(const std::string& name,
                                        const std::vector<std::pair<std::string, std::string>>& field_types);
        void register_builtin_enum(const std::string& name, const std::vector<std::string>& values);

    private:
        struct MemberEntry {
            std::unordered_map<std::string, SymbolPtr> map;
            std::vector<SymbolPtr> list;
        };

        std::unordered_map<std::string, SymbolPtr> globals_{};
        std::unordered_map<std::string, MemberEntry> members_by_owner_{};
        std::vector<std::unordered_map<std::string, SymbolPtr>> scopes_{};
    };
}
