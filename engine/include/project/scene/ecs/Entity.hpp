//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "project/scene/Scene.hpp"
#include "project/scene/ecs/ComponentRegistry.hpp"

namespace engine {
    class Entity {
    public:
        Entity() = default;
        Entity(entt::entity handle, Scene* scene);

        template<typename T, typename... Args>
        T& add_component(Args&&... args);

        template<typename T>
        void remove_component() const;

        template<typename T>
        [[nodiscard]] T& get_component();

        template<typename T>
        [[nodiscard]] bool has_component() const;

        [[nodiscard]] bool is_active() const;
        void set_active(bool active) const;

        UUID uuid();
        const std::string& name();

        explicit operator bool() const { return entity_handle_ != entt::null && scene_ != nullptr; }
        bool operator==(const Entity& other) const { return entity_handle_ == other.entity_handle_ && scene_ == other.scene_; }
        bool operator!=(const Entity& other) const { return !(*this == other); }
        operator entt::entity() const { return entity_handle_; }
        operator uint32_t() const { return static_cast<uint32_t>(entity_handle_); }

    private:
        entt::entity entity_handle_ = entt::null;
        Scene* scene_ = nullptr;
    };

    template<typename T, typename... Args>
    T& Entity::add_component(Args&&... args) {
        if (has_component<T>()) {
            throw std::runtime_error("Engine error: scenes: entity already has component of this type");
        }
        T& component = scene_->registry_.emplace<T>(entity_handle_, std::forward<Args>(args)...);
        ComponentRegistry::register_type<T>();
        return component;
    }

    template<typename T>
    void Entity::remove_component() const {
        if (!has_component<T>()) {
            throw std::runtime_error("Engine error: scenes: entity does not have component of this type");
        }
        scene_->registry_.remove<T>(entity_handle_);
    }

    template<typename T>
    T& Entity::get_component() {
        if (!has_component<T>()) {
            throw std::runtime_error("Engine error: scenes: entity does not have component of this type");
        }
        return scene_->registry_.get<T>(entity_handle_);
    }

    template<typename T>
    bool Entity::has_component() const {
        return scene_->registry_.all_of<T>(entity_handle_);
    }
}
