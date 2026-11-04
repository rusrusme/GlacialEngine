//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

namespace editor {
    enum class EventType {
        // TODO find a way to dynamically use available id (maybe change from enum to ???)
        ActionTriggered = 1024,

        // TODO in future implement more events
        EntitySelected,
        SceneLoaded,
        SceneSaved,
        AssetSelected,
    };
}
