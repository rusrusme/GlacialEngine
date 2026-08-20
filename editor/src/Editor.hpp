#pragma once

#include <core/Engine.hpp>

namespace editor {
    class Editor : public engine::Application {
    public:
        Editor();
        ~Editor() override = default;

    private:
        // Todo: Consider creating a scenes manager
        std::shared_ptr<engine::Scene> active_scene_ = nullptr;
        std::shared_ptr<engine::AssetManager> asset_manager_ = nullptr;
    };
}
