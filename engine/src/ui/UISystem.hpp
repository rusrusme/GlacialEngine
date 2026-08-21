//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <RmlUi/Core.h>

#include "RmlRenderInterface.hpp"
#include "RmlSystemInterface.hpp"
#include "event/Event.hpp"

namespace engine {
    class Renderer;

    class UISystem {
    public:
        UISystem();
        ~UISystem();

        bool init(Renderer& renderer, uint32_t width, uint32_t height);
        void shutdown();

        void on_resize(uint32_t width, uint32_t height);
        void on_update();
        void on_render();
        void on_event(Event& event);

        Rml::ElementDocument* load_document(const std::string& path);

    private:
        RmlSystemInterface system_interface_;
        std::unique_ptr<RmlRenderInterface> render_interface_;
        Rml::Context* context_ = nullptr;
    };
}
