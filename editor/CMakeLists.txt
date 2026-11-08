# editor/CMakeLists.txt
# ----------------------------------------------------------------------------
add_executable(Editor
               src/Editor.cpp
               src/Editor.hpp
               src/CoreLayer.cpp
               src/CoreLayer.hpp

               src/command/Command.hpp
               src/command/CommandHistory.cpp
               src/command/CommandHistory.hpp

               src/ui/UIComponent.hpp
               src/ui/UILayer.cpp
               src/ui/UILayer.hpp
               src/ui/Icons.hpp
               src/ui/panel/AssetBrowser.cpp
               src/ui/panel/AssetBrowser.hpp
               src/ui/panel/ConsolePanel.cpp
               src/ui/panel/ConsolePanel.hpp
               src/ui/panel/DebugPanel.cpp
               src/ui/panel/DebugPanel.hpp
               src/ui/panel/Hierarchy.cpp
               src/ui/panel/Hierarchy.hpp
               src/ui/panel/Inspector.cpp
               src/ui/panel/Inspector.hpp
               src/editor_pch.hpp
               src/Config.hpp
               src/ui/layout/Toolbar.cpp
               src/ui/layout/Toolbar.hpp
               src/ui/layout/Menubar.cpp
               src/ui/layout/Menubar.hpp
               src/ui/window/SettingsWindow.cpp
               src/ui/window/SettingsWindow.hpp
               src/ui/layout/DockspaceLayout.cpp
               src/ui/layout/DockspaceLayout.hpp
               src/event/Shortcuts.cpp
               src/event/Shortcuts.hpp
               src/event/EditorActions.hpp
               src/event/EventType.hpp
               src/event/EditorEvents.hpp
               src/ui/panel/ViewportPanel.cpp
               src/ui/panel/ViewportPanel.hpp
               src/command/CommandBatch.cpp
               src/command/CommandBatch.hpp
               src/command/Transaction.hpp
               src/ui/panel/HistoryPanel.cpp
               src/ui/panel/HistoryPanel.hpp
               src/command/EditorCommands.hpp
               src/command/EntitySnapshot.hpp
               src/rime/RimeEditor.cpp
               src/rime/RimeEditor.hpp
               src/ui/panel/RimeEditorPanel.cpp
               src/ui/panel/RimeEditorPanel.hpp
               src/EditorSettings.hpp
)
# ----------------------------------------------------------------------------
target_include_directories(Editor PRIVATE ${CMAKE_SOURCE_DIR}/editor/src)
target_link_libraries(Editor PRIVATE Engine)
if (ENABLE_IMGUI)
    target_link_libraries(Editor PRIVATE imguizmo::imguizmo)
endif ()

target_precompile_headers(Editor PRIVATE src/editor_pch.hpp)
target_compile_definitions(Editor PRIVATE ENGINE_DIR="${CMAKE_SOURCE_DIR}")

if (TARGET Scripts)
    add_dependencies(Editor Scripts)
endif()
