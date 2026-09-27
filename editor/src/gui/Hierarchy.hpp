//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "scene/Scene.hpp"
#include "scene/ecs/Entity.hpp"

namespace editor {
    enum class EntityType;

    class Hierarchy {
    public:
        Hierarchy() = default;

        void init(engine::SystemContext& context);

        #ifdef ENABLE_IMGUI
        void render_imgui();
        #endif

        engine::Entity add_entity(const std::string& name);

        void update_context(std::shared_ptr<engine::Scene> current_scene);

        [[nodiscard]] engine::Entity get_selected_entity() const { return selected_entity_; }
        void set_selected_entity(const engine::Entity new_entity) { selected_entity_ = new_entity; }

    private:
        std::shared_ptr<engine::Scene> scene_ = nullptr;
        engine::AssetManager* assets = nullptr;
        engine::Entity selected_entity_{};

        char name_buffer_[128] = "";

        #ifdef ENABLE_IMGUI
        void render_header_bar();
        void render_add_entity_popup();
        void render_entity_list(engine::Entity& out_destroy, engine::Entity& out_duplicate);

        void render_primitives_submenus();
        void render_lighting_submenu();
        void render_imported_models_submenu();

        bool menu_item_with_icon(const char* label, EntityType type);
        void create_primitive_entity(const char* label, const char* path, EntityType type);
        std::string resolve_entity_name(const char* default_name) const;

        void render_entity_node(engine::Entity entity, engine::Entity& out_destroy, engine::Entity& out_duplicate);
        bool render_eye_toggle_button(engine::Entity entity, bool is_active);
        void render_entity_context_menu(engine::Entity entity, engine::Entity& out_destroy, engine::Entity& out_duplicate);

        void process_shortcuts(engine::Entity& out_destroy, engine::Entity& out_duplicate);
        void process_deferred_actions(engine::Entity entity_to_destroy, engine::Entity entity_to_duplicate);
        void process_deselection_inputs();
        #endif
    };
}
