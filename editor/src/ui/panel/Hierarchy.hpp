//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "project/scene/ecs/Entity.hpp"
#include "ui/Icons.hpp"
#include "ui/UIComponent.hpp"

namespace editor {
    class Hierarchy : public UIComponent {
    public:
        Hierarchy() : UIComponent("Hierarchy") {}
        ~Hierarchy() override = default;

        void on_update(SystemContext& context, float delta_time) override;
        void on_render_ui(SystemContext& context) override;

        void load_config() override;
        void save_config() override;

        [[nodiscard]] ShortcutContext shortcut_context() const override { return ShortcutContext::Hierarchy; }

        void toggle_visible() { visible_ = !visible_; }
        [[nodiscard]] bool visible() const { return visible_; }
        void set_visible(bool b) { visible_ = b; }

        [[nodiscard]] Entity selected_entity() const { return selected_entity_; }
        void set_selected_entity(Entity new_entity);

        void create_entity();
        void delete_entity();
        void duplicate_entity();
        void rename_entity();
        void deselect_entity();

    private:
        bool visible_ = true;

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
        void render_entity_list(const std::shared_ptr<Scene>& active_scene);

        void render_primitives_submenus(SystemContext& context);
        void render_lighting_submenu(SystemContext& context);
        void render_imported_models_submenu(SystemContext& context);

        bool menu_item_with_icon(const char* label, EntityType type);
        void create_primitive_entity(const char* label, const char* path, EntityType type, SystemContext& context);

        void render_entity_node(Entity entity);
        bool render_eye_toggle_button(bool is_active);
        void render_entity_context_menu(Entity entity);

        void process_deferred_actions(SystemContext& context);
        void process_deselection_inputs();
        void start_renaming(Entity entity);
        void record_entity_creation(Entity entity, SystemContext& context);
    };
}
