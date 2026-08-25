# editor/CMakeLists.txt
# ----------------------------------------------------------------------------
add_executable(Editor
               src/EditorLayer.cpp
               src/EditorLayer.hpp
               src/Editor.hpp
               src/Editor.cpp
               src/environment/EditorGrid.cpp
               src/environment/EditorGrid.hpp
               src/panel/Hierarchy.cpp
               src/panel/Hierarchy.hpp
               src/panel/ObjectInspector.cpp
               src/panel/ObjectInspector.hpp
               src/panel/AssetBrowser.hpp
               src/panel/AssetBrowser.cpp
               src/EditorUiLayer.cpp
               src/EditorUiLayer.hpp
               src/editor_pch.hpp
               src/panel/DebugPanel.cpp
               src/panel/DebugPanel.hpp
               src/panel/Console.cpp
               src/panel/Console.hpp
               src/panel/EntityIcons.hpp
               src/Config.hpp
)
# ----------------------------------------------------------------------------
target_include_directories(Editor PRIVATE ${CMAKE_SOURCE_DIR}/editor/src)

target_link_libraries(Editor PRIVATE Engine)

target_precompile_headers(Editor PRIVATE src/editor_pch.hpp)
