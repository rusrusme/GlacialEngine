//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "project/scene/Scene.hpp"
#include "project/scene/ecs/Entity.hpp"

namespace editor {
    enum class EntityType;

    class Hierarchy {
    public:
        Hierarchy() = default;

        #ifdef ENABLE_IMGUI
        void render_imgui(const engine::SystemContext& context);
        #endif

        void update_context(const std::shared_ptr<engine::Scene>& active_scene);

        [[nodiscard]] engine::Entity get_selected_entity() const { return selected_entity_; }
        void set_selected_entity(engine::Entity new_entity);

    private:
        engine::Entity selected_entity_{};
        engine::UUID selected_uuid_{0};

        engine::Entity renaming_entity_{};
        char rename_buffer_[128] = "";
        bool start_renaming_ = false;

        #ifdef ENABLE_IMGUI
        void render_header_bar();
        void render_add_entity_popup(const engine::SystemContext& context);
        void render_entity_list(engine::Entity& out_destroy, engine::Entity& out_duplicate, const std::shared_ptr<engine::Scene>& active_scene);

        void render_primitives_submenus(const engine::SystemContext& context);
        void render_lighting_submenu(const std::shared_ptr<engine::Scene>& active_scene);
        void render_imported_models_submenu(const engine::SystemContext& context);

        bool menu_item_with_icon(const char* label, EntityType type);
        void create_primitive_entity(const char* label, const char* path, EntityType type, const engine::SystemContext& context);

        void render_entity_node(engine::Entity entity, engine::Entity& out_destroy, engine::Entity& out_duplicate);
        bool render_eye_toggle_button(bool is_active);
        void render_entity_context_menu(engine::Entity entity, engine::Entity& out_destroy, engine::Entity& out_duplicate);

        void process_shortcuts(engine::Entity& out_destroy, engine::Entity& out_duplicate);
        void process_deferred_actions(engine::Entity entity_to_destroy, engine::Entity entity_to_duplicate, const std::shared_ptr<engine::Scene>& active_scene);
        void process_deselection_inputs();
        void rename_entity(engine::Entity entity);
        #endif
    };
}
