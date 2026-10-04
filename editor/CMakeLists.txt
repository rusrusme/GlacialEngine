# editor/CMakeLists.txt
# ----------------------------------------------------------------------------
add_executable(Editor
               src/Editor.cpp
               src/Editor.hpp
               src/EditorLayer.cpp
               src/EditorLayer.hpp
               src/environment/Grid.cpp
               src/environment/Grid.hpp
               src/panel/Hierarchy.cpp
               src/panel/Hierarchy.hpp
               src/panel/ObjectInspector.cpp
               src/panel/ObjectInspector.hpp
               src/panel/AssetBrowser.hpp
               src/panel/AssetBrowser.cpp
               src/gui/UILayer.cpp
               src/gui/UILayer.hpp
               src/editor_pch.hpp
               src/panel/DebugPanel.cpp
               src/panel/DebugPanel.hpp
               src/panel/Console.cpp
               src/panel/Console.hpp
               src/misc/Icons.hpp
               src/Config.hpp
               src/EditorComponents.hpp
               src/gui/Toolbar.cpp
               src/gui/Toolbar.hpp
               src/gui/Menubar.cpp
               src/gui/Menubar.hpp
               src/gui/Settings.cpp
               src/gui/Settings.hpp
               src/misc/StressTests.hpp
               src/ProjectWizard.hpp
               src/ProjectWizard.cpp
               src/command/CommandHistory.cpp
               src/command/CommandHistory.hpp
               src/command/EditorCommands.hpp
               src/command/Command.hpp
               src/command/EditorCommands.cpp
               src/command/EntityCommands.cpp
               src/command/EntityCommands.hpp
)
# ----------------------------------------------------------------------------
target_include_directories(Editor PRIVATE ${CMAKE_SOURCE_DIR}/editor/src)
# ----------------------------------------------------------------------------
target_link_libraries(Editor PRIVATE Engine)
if (ENABLE_IMGUI)
    target_link_libraries(Editor PRIVATE imguizmo::imguizmo)
endif ()
# ----------------------------------------------------------------------------
target_precompile_headers(Editor PRIVATE src/editor_pch.hpp)
# ----------------------------------------------------------------------------
target_compile_definitions(Editor PRIVATE ENGINE_DIR="${CMAKE_SOURCE_DIR}")
