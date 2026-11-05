//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

namespace editor {
    enum class EditorAction {
        // File Actions
        NewScene, OpenScene, SaveScene, SaveSceneAs,
        NewProject, OpenProject, SaveProject, SaveProjectAs,
        Exit,

        // Edit
        Undo, Redo, Copy, Paste,

        // Hierarchy
        DuplicateEntity, DeleteEntity, RenameEntity, DeselectEntity,

        // Runtime
        TogglePlay, // Play, Pause
        StopPlay,

        // Gizmo
        GizmoSelect, GizmoTranslate, GizmoRotate, GizmoScale,
        
        // UI
        ToggleConsole, ToggleInspector, ToggleHierarchy, OpenSettings, ToggleDebug
    };
}