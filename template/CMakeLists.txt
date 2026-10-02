# {template}/CMakeLists.txt
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(TemplateProject
               Game.cpp
               Game.hpp
               GameLayer.cpp
               GameLayer.hpp)

target_link_libraries(TemplateProject PRIVATE Engine)
