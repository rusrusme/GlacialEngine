//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>
#include <vector>

#include "event/Event.hpp"

namespace engine {
    class EventQueue {
    public:
        EventQueue() = delete;

        template<typename T, typename... Args>
        static void submit(Args&&... args) {
            s_queue.push_back(std::make_unique<T>(std::forward<Args>(args)...));
        }

    private:
        friend class Application;

        static void dispatch_all();

        static std::vector<std::unique_ptr<Event>> s_queue;
    };
}
