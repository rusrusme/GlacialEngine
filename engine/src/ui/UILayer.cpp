//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "ui/UILayer.hpp"

#include "core/Application.hpp"
#include "ui/UISystem.hpp"

namespace engine {
    UILayer::UILayer(std::string name) : Layer(std::move(name)), ui_system_(std::make_unique<UISystem>()), manager_(std::make_unique<UIManager>(*ui_system_)) {}

    UILayer::~UILayer() = default;

    void UILayer::on_attach(SystemContext& context) {
        auto width = Application::get().window().width();
        auto height = Application::get().window().height();

        // ui_system_->init(context.renderer, width, height);
    }

    void UILayer::on_detach(SystemContext& context) {
        ui_system_->shutdown();
    }

    void UILayer::on_update(SystemContext& context, float delta_time) {
        ui_system_->on_update();
    }

    void UILayer::on_render(SystemContext& context, float delta_time) {
        ui_system_->on_render();
    }

    void UILayer::on_event(SystemContext& context, Event& event) {
        ui_system_->on_event(event);

        EventDispatcher dispatcher(event);
        dispatcher.dispatch<WindowResizedEvent>([this](const WindowResizedEvent& event) {
            ui_system_->on_resize(event.width(), event.height());
            return false;
        });
    }
}
