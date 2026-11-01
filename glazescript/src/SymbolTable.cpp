//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "SymbolTable.hpp"

#include <ranges>

namespace glaze {
SymbolTable::SymbolTable() {
        register_engine_builtins();
    }

    bool SymbolTable::define_global(const SymbolPtr& symbol) {
        return globals_.emplace(symbol->name, symbol).second;
    }

    SymbolPtr SymbolTable::lookup_global(const std::string& name) const {
        auto it = globals_.find(name);
        return (it != globals_.end()) ? it->second : nullptr;
    }

    void SymbolTable::define_members(const std::string& owner_name,
                                     std::unordered_map<std::string, SymbolPtr> members,
                                     std::vector<SymbolPtr> member_list) {
        if (member_list.empty()) {
            member_list.reserve(members.size());
            for (const auto& sym : members | std::views::values) {
                member_list.push_back(sym);
            }
        }
        members_by_owner_[owner_name] = MemberEntry{.map = std::move(members), .list = std::move(member_list)};
    }

    const std::unordered_map<std::string, SymbolPtr>* SymbolTable::lookup_members(const std::string& owner_name) const {
        auto it = members_by_owner_.find(owner_name);
        return (it != members_by_owner_.end()) ? &it->second.map : nullptr;
    }

    const std::vector<SymbolPtr>* SymbolTable::lookup_member_list(const std::string& owner_name) const {
        auto it = members_by_owner_.find(owner_name);
        return it != members_by_owner_.end() ? &it->second.list : nullptr;
    }

    SymbolPtr SymbolTable::lookup_member(const std::string& owner_name, const std::string& member_name) const {
        auto it = members_by_owner_.find(owner_name);
        if (it == members_by_owner_.end()) return nullptr;
        auto mem_it = it->second.map.find(member_name);
        return mem_it != it->second.map.end() ? mem_it->second : nullptr;
    }

    void SymbolTable::push_scope() {
        scopes_.emplace_back();
    }

    void SymbolTable::pop_scope() {
        if (!scopes_.empty()) {
            scopes_.pop_back();
        }
    }

    bool SymbolTable::define_local(const SymbolPtr& symbol) {
        if (scopes_.empty()) return false;
        return scopes_.back().emplace(symbol->name, symbol).second;
    }

    SymbolPtr SymbolTable::lookup(const std::string& name) const {
        for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
            auto sym_it = it->find(name);
            if (sym_it != it->end()) {
                return sym_it->second;
            }
        }
        return lookup_global(name);
    }

    // Built-ins

    void SymbolTable::register_engine_builtins() {
        register_builtin_enum("BodyType", {"STATIC", "KINEMATIC", "DYNAMIC"});
        register_builtin_enum("AudioTriggerCondition", {"PlayOnAwake", "OnCollisionEnter", "OnTriggerEnter", "Manual"});

        register_builtin_component("LabelComponent", {{"tag", "string"}});
        register_builtin_component("IDComponent", {{"id", "uint64"}});
        register_builtin_component("DisabledComponent", {});

        register_builtin_component("TransformComponent", {
            {"position", "vec3"},
            {"rotation", "vec3"},
            {"scale", "vec3"}
        });

        register_builtin_component("MeshComponent", {{"handle", "uint64"}});

        register_builtin_component("MaterialComponent", {
            {"color", "vec4"},
            {"texture_handle", "uint64"},
            {"normal_texture_handle", "uint64"},
            {"metallic_roughness_texture_handle", "uint64"},
            {"roughness", "float"},
            {"metallic", "float"}
        });

        register_builtin_component("CameraComponent", {
            {"fov", "float"},
            {"near_clip", "float"},
            {"far_clip", "float"},
            {"aspect_ratio", "float"},
            {"is_primary", "bool"}
        });

        register_builtin_component("FlyCameraControllerComponent", {
            {"move_speed", "float"},
            {"sprint_multiplier", "float"},
            {"mouse_sensitivity", "float"},
            {"require_mouse_button", "bool"},
            {"last_mouse_position", "vec2"},
            {"is_first_frame", "bool"}
        });

        register_builtin_component("DirectionalLightComponent", {
            {"color", "vec3"},
            {"intensity", "float"}
        });

        register_builtin_component("PointLightComponent", {
            {"color", "vec3"},
            {"intensity", "float"},
            {"radius", "float"}
        });

        register_builtin_component("RigidBodyComponent", {
            {"type", "BodyType"},
            {"velocity", "vec3"}
        });

        register_builtin_component("BoxColliderComponent", {
            {"center", "vec3"},
            {"size", "vec3"},
            {"is_trigger", "bool"},
            {"show_hitbox", "bool"}
        });

        register_builtin_component("UIComponent", {
            {"rml_filepath", "string"},
            {"is_screen_space", "bool"},
            {"is_visible", "bool"}
        });

        register_builtin_component("AudioSourceComponent", {
            {"sound_handle", "uint64"},
            {"filepath", "string"},
            {"volume", "float"},
            {"pitch", "float"},
            {"loop", "bool"},
            {"is_spatial", "bool"},
            {"min_distance", "float"},
            {"max_distance", "float"},
            {"trigger_condition", "AudioTriggerCondition"},
            {"is_playing", "bool"}
        });

        register_builtin_component("AudioListenerComponent", {{"is_primary", "bool"}});
    }

    void SymbolTable::register_builtin_component(const std::string& name,
                                                 const std::vector<std::pair<std::string, std::string>>& field_types) {
        TypePtr comp_type = Type::make_named(name);
        globals_[name] = std::make_shared<Symbol>(name, SymbolKind::COMPONENT, comp_type, true, "");

        std::unordered_map<std::string, SymbolPtr> fields;
        std::vector<SymbolPtr> field_list;
        for (const auto& [field_name, type_name] : field_types) {
            auto sym = std::make_shared<Symbol>(field_name, SymbolKind::FIELD, Type::make_named(type_name), false, name);
            fields[field_name] = sym;
            field_list.push_back(sym);
        }
        define_members(name, std::move(fields), std::move(field_list));
    }

    void SymbolTable::register_builtin_enum(const std::string& name, const std::vector<std::string>& values) {
        TypePtr enum_type = Type::make_named(name);
        globals_[name] = std::make_shared<Symbol>(name, SymbolKind::ENUM, enum_type, true, "");

        std::unordered_map<std::string, SymbolPtr> members;
        std::vector<SymbolPtr> member_list;
        for (const auto& val : values) {
            auto sym = std::make_shared<Symbol>(val, SymbolKind::ENUM_MEMBER, enum_type, true, name);
            members[val] = sym;
            member_list.push_back(sym);
        }
        define_members(name, std::move(members), std::move(member_list));
    }
}
