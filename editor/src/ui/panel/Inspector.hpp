//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "project/scene/ecs/Entity.hpp"
#include "ui/UIComponent.hpp"

namespace editor {
    class Inspector : public UIComponent {
    public:
        Inspector() : UIComponent("Inspector") {}
        ~Inspector() override = default;

        void on_init(SystemContext& context) override;
        void on_render_ui(SystemContext& context) override;

        void load_config() override;
        void save_config() override;

        [[nodiscard]] ShortcutContext shortcut_context() const override { return ShortcutContext::Inspector; }

    private:
        void draw_component(Entity entity, SystemContext& context) const;

        void draw_header_section(Entity entity) const;
        void draw_transform_component(Entity entity, SystemContext& context) const;
        void draw_mesh_component(Entity entity, SystemContext& context) const;
        void draw_material_component(Entity entity, SystemContext& context) const;
        // void draw_lod_component(Entity entity, const SystemContext& context) const;
        // void draw_lighting_components(Entity entity) const;
        // void draw_box_collider_component(Entity entity, SystemContext& context) const;
        void draw_audio_source_component(Entity entity, SystemContext& context) const;
        void draw_rigid_body_component(Entity entity, SystemContext& context) const;
        void draw_add_component_button(Entity entity, SystemContext& context) const;

        Entity selected_entity_{};
    };
}
