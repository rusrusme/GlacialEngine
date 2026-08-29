//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <string>
#include <unordered_map>
#include <RmlUi/Core.h>

namespace engine {
    class UISystem;

    class UIManager {
    public:
        explicit UIManager(UISystem& ui_system) : ui_system_(ui_system) {}

        Rml::ElementDocument* load_document(const std::string& name, const std::string& filepath);

        void unload_document(const std::string& name);

        void show(const std::string& name);

        void hide(const std::string& name);

        void toggle(const std::string& name);

        [[nodiscard]] Rml::ElementDocument* get_document(const std::string& name);

    private:
        UISystem& ui_system_;
        std::unordered_map<std::string, Rml::ElementDocument*> documents_;
    };
}
