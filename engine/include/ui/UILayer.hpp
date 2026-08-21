//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "../../src/ui/UISystem.hpp"
#include "core/Layer.hpp"
#include "ui/UIManager.hpp"

namespace engine {
    class UISystem;
    class UIManager;

    class UILayer : public Layer {
    public:
        UILayer(std::string name);
        ~UILayer() override;

        void on_attach(Renderer& renderer) override;

        void on_detach() override;

        void on_update(float delta_time) override;

        void on_render(Renderer& renderer, float delta_time) override;

        void on_event(Event& event) override;

        [[nodiscard]] UIManager& get_manager() { return *manager_; }

    private:
        std::unique_ptr<UISystem> ui_system_;
        std::unique_ptr<UIManager> manager_;
    };
}
