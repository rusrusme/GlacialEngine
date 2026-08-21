# editor/CMakeLists.txt
# ----------------------------------------------------------------------------
add_executable(Editor
               src/EditorLayer.cpp
               src/EditorLayer.hpp
               src/Editor.hpp
               src/Editor.cpp
               src/EditorGrid.cpp
               src/EditorGrid.hpp
               src/ui/Hierarchy.cpp
               src/ui/Hierarchy.hpp
               src/ui/ObjectInspector.cpp
               src/ui/ObjectInspector.hpp
               src/ui/AssetBrowser.hpp
               src/ui/AssetBrowser.cpp
               src/EditorUiLayer.cpp
               src/EditorUiLayer.hpp
               src/physics/Slab.hpp
               src/editor_pch.hpp
)
# ----------------------------------------------------------------------------
target_include_directories(Editor PRIVATE ${CMAKE_SOURCE_DIR}/editor/src)

target_link_libraries(Editor PRIVATE Engine)

target_precompile_headers(Editor PRIVATE src/editor_pch.hpp)
