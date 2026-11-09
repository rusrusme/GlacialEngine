//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>
#include <string_view>

#include "command/Command.hpp"
#include "command/EntitySnapshot.hpp"
#include "nlohmann/json_fwd.hpp"
#include "project/scene/Scene.hpp"
#include "project/scene/SceneManager.hpp"
#include "project/scene/ecs/Entity.hpp"

namespace editor {
    using namespace engine;

    // Helper to find an entity by its immutable UUID across active scene views
    inline Entity find_entity_by_uuid(const std::shared_ptr<Scene>& scene, UUID uuid) {
        if (!scene) return {};
        auto view = scene->view<IDComponent>();
        for (const auto& [entity, id] : view.each()) {
            if (id.id == uuid) {
                return {entity, scene.get()};
            }
        }
        return {};
    }

    // --- CREATING ENTITY COMMAND ---
    class CreateEntityCommand : public Command {
    public:
        CreateEntityCommand(Entity entity, SystemContext& context)
            : entity_uuid_(entity.uuid()),
              snapshot_(EntitySnapshot::serialize(entity, context)) {}

        explicit CreateEntityCommand(nlohmann::json snapshot)
            : entity_uuid_(snapshot.value("id", static_cast<uint64_t>(0))),
              snapshot_(std::move(snapshot)) {}

        void execute(SystemContext& context) override {
            auto scene = context.scenes.active_scene();
            if (!scene) return;
            // Prevent duplicate instantiation if already present in scene
            if (find_entity_by_uuid(scene, entity_uuid_)) {
                return;
            }
            EntitySnapshot::deserialize(snapshot_, context);
        }

        void undo(SystemContext& context) override {
            auto scene = context.scenes.active_scene();
            if (Entity entity = find_entity_by_uuid(scene, entity_uuid_)) {
                scene->destroy_entity(entity);
            }
        }

        [[nodiscard]] UUID entity_uuid() const noexcept { return entity_uuid_; }
        [[nodiscard]] std::string_view name() const override { return "Create Entity"; }

        [[nodiscard]] std::unique_ptr<Command> clone() const override {
            return std::make_unique<CreateEntityCommand>(*this);
        }

    private:
        UUID entity_uuid_;
        nlohmann::json snapshot_;
    };

    // --- DELETE ENTITY COMMAND ---
    class DeleteEntityCommand : public Command {
    public:
        DeleteEntityCommand(Entity entity, SystemContext& context)
            : entity_uuid_(entity.uuid()),
              snapshot_(EntitySnapshot::serialize(entity, context)) {}

        void execute(SystemContext& context) override {
            auto scene = context.scenes.active_scene();
            if (Entity entity = find_entity_by_uuid(scene, entity_uuid_)) {
                scene->destroy_entity(entity);
            }
        }

        void undo(SystemContext& context) override {
            auto scene = context.scenes.active_scene();
            if (!scene) return;
            EntitySnapshot::deserialize(snapshot_, context);
        }

        [[nodiscard]] std::string_view name() const override { return "Delete Entity"; }

        [[nodiscard]] std::unique_ptr<Command> clone() const override {
            return std::make_unique<DeleteEntityCommand>(*this);
        }

    private:
        UUID entity_uuid_;
        nlohmann::json snapshot_;
    };

    // --- DUPLICATE ENTITY COMMAND ---
    class DuplicateEntityCommand : public Command {
    public:
        DuplicateEntityCommand(Entity source, SystemContext& context)
            : source_uuid_(source.uuid()),
              duplicate_uuid_(UUID()),
              snapshot_(EntitySnapshot::serialize(source, context)) {
            // Assign the deterministic new UUID to the snapshot
            snapshot_["id"] = static_cast<uint64_t>(duplicate_uuid_);
            std::string label = snapshot_.value("label", "Entity");
            snapshot_["label"] = label + " (Copy)";
        }

        void execute(SystemContext& context) override {
            auto scene = context.scenes.active_scene();
            if (!scene) return;
            // Spawn the duplicate from snapshot
            EntitySnapshot::deserialize(snapshot_, context);
        }

        void undo(SystemContext& context) override {
            auto scene = context.scenes.active_scene();
            if (Entity entity = find_entity_by_uuid(scene, duplicate_uuid_)) {
                scene->destroy_entity(entity);
            }
        }

        [[nodiscard]] UUID duplicate_uuid() const noexcept { return duplicate_uuid_; }
        [[nodiscard]] std::string_view name() const override { return "Duplicate Entity"; }

        [[nodiscard]] std::unique_ptr<Command> clone() const override {
            return std::make_unique<DuplicateEntityCommand>(*this);
        }

    private:
        UUID source_uuid_;
        UUID duplicate_uuid_;
        nlohmann::json snapshot_;
    };

    // --- RENAME ENTITY COMMAND ---
    class RenameEntityCommand : public Command {
    public:
        RenameEntityCommand(UUID uuid, std::string old_name, std::string new_name)
            : entity_uuid_(uuid),
              old_name_(std::move(old_name)),
              new_name_(std::move(new_name)) {}

        void execute(SystemContext& context) override {
            auto scene = context.scenes.active_scene();
            if (Entity entity = find_entity_by_uuid(scene, entity_uuid_)) {
                entity.get_component<LabelComponent>().tag = new_name_;
            }
        }

        void undo(SystemContext& context) override {
            auto scene = context.scenes.active_scene();
            if (Entity entity = find_entity_by_uuid(scene, entity_uuid_)) {
                entity.get_component<LabelComponent>().tag = old_name_;
            }
        }

        [[nodiscard]] std::string_view name() const override { return "Rename Entity"; }

        [[nodiscard]] std::unique_ptr<Command> clone() const override {
            return std::make_unique<RenameEntityCommand>(*this);
        }

    private:
        UUID entity_uuid_;
        std::string old_name_;
        std::string new_name_;
    };

    // --- CHANGE TRANSFORM COMMAND ---
    class ChangeTransformCommand : public Command {
    public:
        ChangeTransformCommand(UUID uuid, TransformComponent old_transform, TransformComponent new_transform, bool mergeable = false)
            : entity_uuid_(uuid),
              old_transform_(old_transform),
              new_transform_(new_transform),
              mergeable_(mergeable) {}

        void execute(SystemContext& context) override {
            auto scene = context.scenes.active_scene();
            if (Entity entity = find_entity_by_uuid(scene, entity_uuid_)) {
                if (entity.has_component<TransformComponent>()) {
                    entity.get_component<TransformComponent>() = new_transform_;
                }
            }
        }

        void undo(SystemContext& context) override {
            auto scene = context.scenes.active_scene();
            if (Entity entity = find_entity_by_uuid(scene, entity_uuid_)) {
                if (entity.has_component<TransformComponent>()) {
                    entity.get_component<TransformComponent>() = old_transform_;
                }
            }
        }

        bool merge(const Command& next) override {
            if (!mergeable_) {
                return false;
            }

            auto* other = dynamic_cast<const ChangeTransformCommand*>(&next);
            if (!other || !other->mergeable_ || other->entity_uuid_ != entity_uuid_) {
                return false;
            }
            new_transform_ = other->new_transform_;
            return true;
        }

        [[nodiscard]] std::string_view name() const override { return "Transform Entity"; }

        [[nodiscard]] std::unique_ptr<Command> clone() const override {
            return std::make_unique<ChangeTransformCommand>(*this);
        }

    private:
        UUID entity_uuid_;
        TransformComponent old_transform_;
        TransformComponent new_transform_;
        bool mergeable_{false};
    };

    // --- UPDATE ENTITY COMMAND ---
    class UpdateEntityCommand : public Command {
    public:
        UpdateEntityCommand(UUID uuid, nlohmann::json old_snapshot, nlohmann::json new_snapshot, std::string name = "Update Entity")
            : entity_uuid_(uuid),
              old_snapshot_(std::move(old_snapshot)),
              new_snapshot_(std::move(new_snapshot)),
              name_(std::move(name)) {}

        void execute(SystemContext& context) override {
            auto scene = context.scenes.active_scene();
            if (find_entity_by_uuid(scene, entity_uuid_)) {
                EntitySnapshot::deserialize(new_snapshot_, context);
            }
        }

        void undo(SystemContext& context) override {
            auto scene = context.scenes.active_scene();
            if (find_entity_by_uuid(scene, entity_uuid_)) {
                EntitySnapshot::deserialize(old_snapshot_, context);
            }
        }

        bool merge(const Command& next) override {
            auto* other = dynamic_cast<const UpdateEntityCommand*>(&next);
            if (!other || other->entity_uuid_ != entity_uuid_ || other->name_ != name_) {
                return false;
            }
            new_snapshot_ = other->new_snapshot_;
            return true;
        }

        [[nodiscard]] std::string_view name() const override { return name_; }

        [[nodiscard]] std::unique_ptr<Command> clone() const override {
            return std::make_unique<UpdateEntityCommand>(*this);
        }

    private:
        UUID entity_uuid_;
        nlohmann::json old_snapshot_;
        nlohmann::json new_snapshot_;
        std::string name_;
    };
}
