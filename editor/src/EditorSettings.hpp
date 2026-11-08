//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>
#include <string_view>
#include <typeindex>
#include <unordered_map>

namespace engine {
    struct SystemContext;
}

namespace editor {
    using namespace engine;

    class SettingsCategory {
    public:
        virtual ~SettingsCategory() = default;

        [[nodiscard]] virtual std::string_view name() const = 0;
        virtual void on_render_ui(SystemContext& context) = 0;

        virtual void load() = 0;
        virtual void save() = 0;
    };

    class EditorSettings {
    public:
        template<typename T, typename... Args>
        T& register_category(Args&&... args) {
            const auto type = std::type_index(typeid(T));
            auto instance = std::make_shared<T>(std::forward<Args>(args)...);
            settings_[type] = instance;
            categories_.push_back(instance);
            instance->load();
            return *instance;
        }

        template<typename T>
        [[nodiscard]] T& get() {
            const auto it = settings_.find(std::type_index(typeid(T)));
            assert(it != settings_.end() && "Requested EditorSettings category does not exist!");
            return *std::static_pointer_cast<T>(it->second);
        }

        template<typename T>
        [[nodiscard]] bool has() const noexcept {
            return settings_.contains(std::type_index(typeid(T)));
        }

        void load_all() {
            for (auto& cat : categories_) cat->load();
        }

        void save_all() {
            for (auto& cat : categories_) cat->save();
        }

        [[nodiscard]] const std::vector<std::shared_ptr<SettingsCategory>>& categories() const noexcept {
            return categories_;
        }

    private:
        std::unordered_map<std::type_index, std::shared_ptr<SettingsCategory>> settings_;
        std::vector<std::shared_ptr<SettingsCategory>> categories_;
    };
}
