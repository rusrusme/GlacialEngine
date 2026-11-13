cmake_minimum_required(VERSION 3.21)

project(GlacialEngine CXX C)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (PROJECT_IS_TOP_LEVEL)
    option(GLACIAL_ENGINE_ONLY "Build only the core Engine library" OFF)
else ()
    option(GLACIAL_ENGINE_ONLY "Build only the core Engine library" ON)
endif ()

option(ENABLE_IMGUI "Include ImGui Dependency" ON)
option(ENABLE_SANDBOX "Build Sandbox demo project" ${PROJECT_IS_TOP_LEVEL})

if (ENABLE_IMGUI)
    add_compile_definitions(ENABLE_IMGUI)
endif ()

if (NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif ()
if (NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif ()
if (NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif ()

include(Dependencies.cmake)

set(CMAKE_FOLDER "GlacialEngine")
add_subdirectory(glazescript)
add_subdirectory(gshader)
unset(CMAKE_FOLDER)

add_subdirectory(engine)

if (NOT GLACIAL_ENGINE_ONLY)
    add_subdirectory(editor)

    set(CMAKE_FOLDER "Tests")
    add_subdirectory(test)
    unset(CMAKE_FOLDER)

    if (ENABLE_SANDBOX)
        add_subdirectory(sandbox)
    endif ()
endif ()