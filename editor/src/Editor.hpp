#pragma once

#include <core/Engine.hpp>

#include "EditorLayer.hpp"

namespace editor {
    class Editor : public engine::Application {
    public:
        Editor();
        ~Editor() override = default;
    };
}
