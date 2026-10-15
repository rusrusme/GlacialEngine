//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "core/Layer.hpp"
#include "ui/UIManager.hpp"

namespace engine {
    class UISystem;
    class UIManager;

    class UILayer : public Layer {
    public:
        UILayer(std::string name);
        ~UILayer() override;

        void on_attach(SystemContext& context) override;
        void on_detach(SystemContext& context) override;

        void on_update(SystemContext& context, float delta_time) override;
        void on_render(SystemContext& context, float delta_time) override;
        void on_event(SystemContext& context, Event& event) override;

        [[nodiscard]] UIManager& manager() { return *manager_; }

    private:
        std::unique_ptr<UISystem> ui_system_;
        std::unique_ptr<UIManager> manager_;
    };
}
