//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "command/Command.hpp"

// Todo
namespace editor {
    class RenameEntityCommand : public Command {
    public:
        RenameEntityCommand(engine::Entity entity, std::string old_name, std::string new_name)
            : entity_(entity), old_name_(std::move(old_name)), new_name_(std::move(new_name)) {}

        void execute() override;
        void undo() override;
        [[nodiscard]] std::string_view get_name() const override { return "Rename Entity"; }

    private:
        engine::Entity entity_;
        std::string old_name_;
        std::string new_name_;
    };

    class CreateEntityCommand : public Command {
    public:
        CreateEntityCommand(std::shared_ptr<engine::Scene> scene, std::string name = "Empty Entity")
            : scene_(std::move(scene)), name_(std::move(name)) {}

        void execute() override {
            if (serialized_data_.empty()) {
                // First execution: Create fresh entity
                created_entity_ = scene_->create_entity(name_);
                uuid_ = created_entity_.uuid();
            } else {
                // Redo: Restore entity with preserved UUID and components
                // created_entity_ = engine::SceneSerializer::deserialize_entity(scene_.get(), serialized_data_);
            }
        }

        void undo() override {
            if (created_entity_) {
                // Backup full entity component state before destroying
                // serialized_data_ = engine::SceneSerializer::serialize_entity(created_entity_);
                scene_->destroy_entity(created_entity_);
                created_entity_ = {};
            }
        }

        [[nodiscard]] engine::Entity get_entity() const { return created_entity_; }
        [[nodiscard]] std::string_view get_name() const override { return "Create Entity"; }

    private:
        std::shared_ptr<engine::Scene> scene_;
        engine::Entity created_entity_{};
        engine::UUID uuid_{};
        std::string name_;
        std::string serialized_data_; // Snapshot for Redo
    };

    class DestroyEntityCommand : public Command {
    public:
        DestroyEntityCommand(std::shared_ptr<engine::Scene> scene, engine::Entity entity)
            : scene_(std::move(scene)), entity_(entity) {
            if (entity_) {
                // Snapshot entity state during command construction before execute() is called
                // serialized_data_ = engine::SceneSerializer::serialize_entity(entity_);
                uuid_ = entity_.uuid();
            }
        }

        void execute() override {
            if (entity_) {
                scene_->destroy_entity(entity_);
                entity_ = {};
            }
        }

        void undo() override {
            // Restore entity with exact same UUID and components
            // entity_ = engine::SceneSerializer::deserialize_entity(scene_.get(), serialized_data_);
        }

        [[nodiscard]] std::string_view get_name() const override { return "Destroy Entity"; }

    private:
        std::shared_ptr<engine::Scene> scene_;
        engine::Entity entity_{};
        engine::UUID uuid_{};
        std::string serialized_data_; // Snapshot for Undo
    };
}
