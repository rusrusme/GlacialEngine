//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <string>
#include <string_view>

#include "command/Command.hpp"
#include "project/scene/ecs/Entity.hpp"

namespace editor {
    class ChangeTransformCommand : public Command {
    public:
        ChangeTransformCommand(engine::Entity entity, engine::TransformComponent old_transform, engine::TransformComponent new_transform)
            : entity_(entity), old_transform_(old_transform), new_transform_(new_transform) {}

        void execute() override;
        void undo() override;
        [[nodiscard]] std::string_view get_name() const override { return "Change Transform"; }

    private:
        engine::Entity entity_;
        engine::TransformComponent old_transform_;
        engine::TransformComponent new_transform_;
    };

    template<typename T>
    class SetComponentCommand : public Command {
    public:
        SetComponentCommand(engine::Entity entity, T old_value, T new_value, std::string_view name = "Modify Component")
            : entity_(entity), old_value_(std::move(old_value)), new_value_(std::move(new_value)), name_(name) {
            if (entity_) {
                had_component_originally_ = entity_.has_component<T>();
            }
        }

        void execute() override {
            if (!entity_) return;

            if (entity_.has_component<T>()) {
                entity_.get_component<T>() = new_value_;
            }
            else {
                entity_.add_component<T>(new_value_);
            }
        }

        void undo() override {
            if (!entity_) return;

            if (had_component_originally_) {
                if (entity_.has_component<T>()) {
                    entity_.get_component<T>() = old_value_;
                }
                else {
                    entity_.add_component<T>(old_value_);
                }
            }
            else {
                if (entity_.has_component<T>()) {
                    entity_.remove_component<T>();
                }
            }
        }

        bool merge_with(const Command* next) override {
            auto* next_cmd = dynamic_cast<const SetComponentCommand<T>*>(next);
            if (next_cmd && next_cmd->entity_ == entity_) {
                new_value_ = next_cmd->new_value_;
                return true;
            }
            return false;
        }

        [[nodiscard]] std::string_view get_name() const override { return name_; }

    private:
        engine::Entity entity_;
        T old_value_{};
        T new_value_{};
        bool had_component_originally_ = false;
        std::string name_;
    };
}
