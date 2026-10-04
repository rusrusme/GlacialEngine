//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "audio/AudioSystem.hpp"
#include "project/scene/ecs/Entity.hpp"

namespace editor {
    class ObjectInspector {
    public:
        ObjectInspector() = default;

        #ifdef ENABLE_IMGUI
        void render_imgui(engine::SystemContext& context);
        #endif

        void set_selected_entity(const engine::Entity entity) { selected_entity_ = entity; }

    private:
        #ifdef ENABLE_IMGUI
        void draw_component(engine::Entity entity, engine::SystemContext& context) const;

        void draw_header_section(engine::Entity entity) const;
        void draw_transform_component(engine::Entity entity) const;
        void draw_mesh_component(engine::Entity entity, engine::SystemContext& context) const;
        void draw_material_component(engine::Entity entity, engine::SystemContext& context) const;
        void draw_lod_component(engine::Entity entity, const engine::SystemContext& context) const;
        void draw_lighting_components(engine::Entity entity) const;
        void draw_box_collider_component(engine::Entity entity, engine::SystemContext& context) const;
        void draw_audio_source_component(engine::Entity entity, engine::SystemContext& context) const;
        void draw_add_component_button(engine::Entity entity, engine::SystemContext& context) const;
        #endif

        engine::Entity selected_entity_{};
    };
}
