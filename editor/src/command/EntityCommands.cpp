//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "EntityCommands.hpp"

// Todo
namespace editor {
    void RenameEntityCommand::execute() {
        if (entity_ && entity_.has_component<engine::LabelComponent>()) {
            entity_.get_component<engine::LabelComponent>().tag = new_name_;
        }
    }

    void RenameEntityCommand::undo() {
        if (entity_ && entity_.has_component<engine::LabelComponent>()) {
            entity_.get_component<engine::LabelComponent>().tag = old_name_;
        }
    }
}