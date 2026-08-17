//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Editor.hpp"

namespace editor {
    Editor::Editor() {
        push_layer(std::make_unique<EditorLayer>());
    }
}

int main() {
    editor::Editor engine;
    return engine.run();
}
