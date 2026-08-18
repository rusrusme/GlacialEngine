#pragma once

#include <core/Engine.hpp>

namespace editor {
    class Editor : public engine::Application {
    public:
        Editor();
        ~Editor() override = default;
    private:
        // Todo: Consider creating a scene manager
        std::shared_ptr<engine::Scene> active_scene_;
    };
}
