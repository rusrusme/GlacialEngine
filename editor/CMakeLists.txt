# editor/CMakeLists.txt
# ----------------------------------------------------------------------------
add_executable(Editor
               src/Editor.cpp
               src/Editor.hpp
               src/CoreLayer.cpp
               src/CoreLayer.hpp

               src/command/Command.hpp

               src/ui/UIComponent.hpp
               src/ui/UILayer.cpp
               src/ui/UILayer.hpp
               src/ui/Icons.hpp
               src/ui/AssetBrowser.cpp
               src/ui/AssetBrowser.hpp
               src/ui/Console.cpp
               src/ui/Console.hpp
               src/ui/DebugPanel.cpp
               src/ui/DebugPanel.hpp
               src/ui/Hierarchy.cpp
               src/ui/Hierarchy.hpp
               src/ui/Inspector.cpp
               src/ui/Inspector.hpp
               src/editor_pch.hpp
)
# ----------------------------------------------------------------------------
target_include_directories(Editor PRIVATE ${CMAKE_SOURCE_DIR}/editor/src)
target_link_libraries(Editor PRIVATE Engine)
if (ENABLE_IMGUI)
    target_link_libraries(Editor PRIVATE imguizmo::imguizmo)
endif ()

target_precompile_headers(Editor PRIVATE src/editor_pch.hpp)
target_compile_definitions(Editor PRIVATE ENGINE_DIR="${CMAKE_SOURCE_DIR}")
