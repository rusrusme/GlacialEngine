# editor/CMakeLists.txt
add_executable(Editor
               src/EditorLayer.cpp
               src/EditorLayer.hpp
               src/Editor.hpp
               src/Editor.cpp
               src/EditorGrid.cpp
               src/EditorGrid.hpp

               src/ui/AssetBrowser.cpp
               src/ui/AssetBrowser.hpp
               src/ui/Console.cpp
               src/ui/Console.hpp
               src/ui/Hierarchy.cpp
               src/ui/Hierarchy.hpp
               src/ui/ObjectInspector.cpp
               src/ui/ObjectInspector.hpp
               src/ui/PlacementToolbar.cpp
               src/ui/PlacementToolbar.hpp
               src/EditorUiLayer.cpp
               src/EditorUiLayer.hpp
)

target_include_directories(Editor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(Editor PRIVATE Engine)