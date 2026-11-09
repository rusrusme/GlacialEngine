//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "project/scene/ecs/Entity.hpp"
#include "ui/Icons.hpp"
#include "ui/UIComponent.hpp"

namespace editor {
    struct SelectionContext {
        Entity selected_entity{};
        UUID selected_uuid{0};

        void set_selected(Entity entity) {
            selected_entity = entity;
            if (selected_entity && selected_entity.has_component<IDComponent>()) {
                selected_uuid = selected_entity.get_component<IDComponent>().id;
            } else {
                selected_uuid = 0;
            }
        }

        void clear() {
            selected_entity = {};
            selected_uuid = 0;
        }
    };

    class Hierarchy : public UIComponent {
    public:
        Hierarchy() : UIComponent("Hierarchy") {}
        ~Hierarchy() override = default;

        void on_update(SystemContext& context, float delta_time) override;
        void on_render_ui(SystemContext& context) override;

        void load_config() override;
        void save_config() override;

        [[nodiscard]] ShortcutContext shortcut_context() const override { return ShortcutContext::Hierarchy; }

        [[nodiscard]] Entity selected_entity() const { return selected_entity_; }
        void set_selected_entity(SystemContext& context, Entity new_entity);

        void create_entity();
        void delete_entity(SystemContext& context);
        void duplicate_entity(SystemContext& context);
        void rename_entity(SystemContext& context);
        void deselect_entity(SystemContext& context);

    private:
        Entity selected_entity_;
        UUID selected_uuid_;

        Entity renaming_entity_;
        char rename_buffer_[128] = "";
        bool renaming_ = false;

        bool create_entity_requested_{false};
        Entity entity_to_destroy_{};
        Entity entity_to_duplicate_{};
        Entity entity_to_rename_{};
        std::string new_entity_name_{};

    private:
        void render_header_bar();
        void render_add_entity_popup(SystemContext& context);
        void render_entity_list(SystemContext& context);

        void render_primitives_submenus(SystemContext& context);
        void render_lighting_submenu(SystemContext& context);
        void render_imported_models_submenu(SystemContext& context);

        bool menu_item_with_icon(const char* label, EntityType type);
        void create_primitive_entity(const char* label, const char* path, EntityType type, SystemContext& context);

        void render_entity_node(SystemContext& context, Entity entity);
        bool render_eye_toggle_button(bool is_active);
        void render_entity_context_menu(Entity entity);

        void process_deferred_actions(SystemContext& context);
        void process_deselection_inputs(SystemContext& context);
        void start_renaming(Entity entity);
        void record_entity_creation(Entity entity, SystemContext& context);
    };
}
