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
}
