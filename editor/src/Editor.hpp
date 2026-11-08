//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "CoreLayer.hpp"
#include "EditorSettings.hpp"
#include "command/CommandHistory.hpp"
#include "core/Application.hpp"
#include "rime/RimeEditor.hpp"
#include "ui/UILayer.hpp"

namespace editor {
    using namespace engine;

    class Editor : public Application {
    public:
        Editor() : Application("Glacial Engine") {
            push_layer(std::make_unique<CoreLayer>());
            push_layer(std::make_unique<RimeEditor>());
            push_overlay(std::make_unique<UILayer>());
        }

        ~Editor() override = default;

    protected:
        void init(SystemContext& context) override {
            auto& settings = context.emplace<EditorSettings>();
            settings.register_category<RimeSettings>();

            context.emplace<RimeContext>();
            context.emplace<SceneViewport>();
            context.emplace<CommandHistory>();
        }
    };
}
