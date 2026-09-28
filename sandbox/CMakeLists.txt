# sandbox/CMakeLists.txt
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(Sandbox
               Sandbox.cpp
               Sandbox.hpp
               SandboxLayer.cpp
               SandboxLayer.hpp)

target_link_libraries(Sandbox PRIVATE Engine)
