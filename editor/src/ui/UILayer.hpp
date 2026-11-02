//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "core/Layer.hpp"
#include "debug/ImguiLayer.hpp"

namespace editor {
    using namespace engine;

    class UILayer : public ImguiLayer {
    public:
        UILayer() : ImguiLayer("Editor UI Layer") {}
        ~UILayer() override = default;

    protected:
        void on_imgui_render(SystemContext& context, float delta_time) override;
    };
}
