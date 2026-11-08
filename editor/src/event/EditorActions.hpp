//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

namespace editor {
    enum class Action {
        // File Actions
        NewScene, OpenScene, SaveScene, SaveSceneAs,
        NewProject, OpenProject, SaveProject, SaveProjectAs,
        Exit,

        // Edit
        Undo, Redo, Copy, Paste,

        // Hierarchy
        DuplicateEntity, DeleteEntity, RenameEntity, DeselectEntity, CreateEntity,

        // Runtime
        TogglePlay, // Play, Pause
        StopPlay,

        // Gizmo
        GizmoSelect, GizmoTranslate, GizmoRotate, GizmoScale,

        // UI
        ToggleConsole, ToggleInspector, ToggleHierarchy, ToggleSettings,
        ToggleDebug, ToggleHistory, ToggleAssetBrowser, ToggleViewport, ToggleRime
    };
}
