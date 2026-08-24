//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "ui/UIManager.hpp"

#include "ui/UISystem.hpp"

namespace engine {
    Rml::ElementDocument* UIManager::load_document(const std::string& name, const std::string& filepath) {
        if (documents_.contains(name)) {
            return documents_[name];
        }

        if (auto* doc = ui_system_.load_document(filepath)) {
            documents_[name] = doc;
            return doc;
        }
        return nullptr;
    }

    void UIManager::unload_document(const std::string& name) {
        auto it = documents_.find(name);
        if (it != documents_.end()) {
            if (it->second) {
                it->second->Close();
            }
            documents_.erase(it);
        }
    }

    void UIManager::show(const std::string& name) {
        if (auto it = documents_.find(name); it != documents_.end() && it->second) {
            it->second->Show();
        }
    }

    void UIManager::hide(const std::string& name) {
        if (auto it = documents_.find(name); it != documents_.end() && it->second) {
            it->second->Hide();
        }
    }

    void UIManager::toggle(const std::string& name) {
        if (auto it = documents_.find(name); it != documents_.end() && it->second) {
            if (it->second->IsVisible()) {
                it->second->Hide();
            }
            else {
                it->second->Show();
            }
        }
    }

    Rml::ElementDocument* UIManager::get_document(const std::string& name) {
        auto it = documents_.find(name);
        return (it != documents_.end()) ? it->second : nullptr;
    }
}
