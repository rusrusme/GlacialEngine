//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "command/EditorCommands.hpp"

namespace editor {
    void ChangeTransformCommand::execute() {
        if (entity_ && entity_.has_component<engine::TransformComponent>()) {
            entity_.get_component<engine::TransformComponent>() = new_transform_;
        }
    }

    void ChangeTransformCommand::undo() {
        if (entity_ && entity_.has_component<engine::TransformComponent>()) {
            entity_.get_component<engine::TransformComponent>() = old_transform_;
        }
    }
}
