# editor/CMakeLists.txt
add_executable(Editor
               src/EditorLayer.cpp
               src/EditorLayer.hpp
               src/Editor.hpp
               src/Editor.cpp
               src/EditorGrid.cpp
               src/EditorGrid.hpp
)

target_include_directories(Editor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(Editor PRIVATE Engine)