//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "CoreLayer.hpp"

namespace editor {
    void CoreLayer::on_attach(SystemContext& context) {
        Layer::on_attach(context);
    }

    void CoreLayer::on_detach(SystemContext& context) {
        Layer::on_detach(context);
    }

    void CoreLayer::on_update(SystemContext& context, float delta_time) {
        Layer::on_update(context, delta_time);
    }

    void CoreLayer::on_render(SystemContext& context, float delta_time) {
        Layer::on_render(context, delta_time);
    }

    void CoreLayer::on_event(SystemContext& context, Event& event) {
        Layer::on_event(context, event);
    }
}
