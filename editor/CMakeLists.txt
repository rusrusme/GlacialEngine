# editor/CMakeLists.txt
# ----------------------------------------------------------------------------
add_executable(Editor
               src/EditorLayer.cpp
               src/EditorLayer.hpp
               src/Editor.hpp
               src/Editor.cpp
               src/environment/Grid.cpp
               src/environment/Grid.hpp
               src/gui/Hierarchy.cpp
               src/gui/Hierarchy.hpp
               src/gui/ObjectInspector.cpp
               src/gui/ObjectInspector.hpp
               src/gui/AssetBrowser.hpp
               src/gui/AssetBrowser.cpp
               src/gui/UILayer.cpp
               src/gui/UILayer.hpp
               src/editor_pch.hpp
               src/gui/DebugPanel.cpp
               src/gui/DebugPanel.hpp
               src/gui/Console.cpp
               src/gui/Console.hpp
               src/gui/EntityIcons.hpp
               src/Config.hpp
               src/EditorComponents.hpp
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
