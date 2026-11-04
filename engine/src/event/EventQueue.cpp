//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "event/EventQueue.hpp"

#include "core/Application.hpp"

// TODO beware of implementation. May be buggy
namespace engine {
    std::vector<std::unique_ptr<Event>> EventQueue::s_queue;

    void EventQueue::dispatch_all() {
        if (s_queue.empty()) return;

        std::vector<std::unique_ptr<Event>> local_queue;
        s_queue.swap(local_queue);

        for (auto& event : local_queue) {
            Application::get().on_event(*event);
        }
    }
}
