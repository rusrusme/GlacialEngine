//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "UISystem.hpp"

#include <RmlUi/Debugger/Debugger.h>

#include "event/MouseEvents.hpp"
#include "render/Renderer.hpp"

namespace engine {
    UISystem::UISystem() = default;

    UISystem::~UISystem() = default;

    bool UISystem::init(Renderer& renderer, uint32_t width, uint32_t height) {
        render_interface_ = std::make_unique<RmlRenderInterface>(renderer);
        render_interface_->set_viewport(width, height);

        renderer.get_shader_library().load("ui", "assets/shaders/engine/ui.vert", "assets/shaders/engine/ui.frag");

        Rml::SetSystemInterface(&system_interface_);
        Rml::SetRenderInterface(render_interface_.get());

        if (!Rml::Initialise()) return false;

        Rml::LoadFontFace("assets/fonts/Inter-VariableFont.ttf");

        context_ = Rml::CreateContext("main_context", Rml::Vector2i(width, height));
        if (!context_) return false;

        Rml::Debugger::Initialise(context_);
        return true;
    }

    void UISystem::shutdown() {
        if (context_) {
            Rml::Shutdown();
            context_ = nullptr;
        }
    }

    void UISystem::on_resize(uint32_t width, uint32_t height) {
        if (context_) {
            context_->SetDimensions(Rml::Vector2i(width, height));
        }
        if (render_interface_) {
            render_interface_->set_viewport(width, height);
        }
    }

    void UISystem::on_update() {
        if (context_) {
            context_->Update();
        }
    }

    void UISystem::on_render() {
        if (context_) {
            Renderer::set_blend(true); // Ensure alpha blending is enabled
            glDisable(GL_DEPTH_TEST);
            context_->Render();
            glEnable(GL_DEPTH_TEST);
        }
    }

    void UISystem::on_event(Event& event) {
        if (!context_) return;

        EventDispatcher dispatcher(event);

        dispatcher.dispatch<MouseMovedEvent>([this](const MouseMovedEvent& e) {
            context_->ProcessMouseMove(static_cast<int>(e.x()), static_cast<int>(e.y()), 0);
            return false;
        });

        dispatcher.dispatch<MouseButtonPressedEvent>([this](const MouseButtonPressedEvent& e) {
            context_->ProcessMouseButtonDown(e.button(), 0);
            return false;
        });

        dispatcher.dispatch<MouseButtonReleasedEvent>([this](const MouseButtonReleasedEvent& e) {
            context_->ProcessMouseButtonUp(e.button(), 0);
            return false;
        });
    }

    Rml::ElementDocument* UISystem::load_document(const std::string& path) {
        if (!context_) return nullptr;
        auto* doc = context_->LoadDocument(path);
        if (doc) doc->Show();
        return doc;
    }
}
