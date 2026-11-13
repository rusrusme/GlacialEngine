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

option(ENGINE_DEBUG "Engine-only debug distribution build" OFF)
option(ENGINE_RELEASE "Engine-only release distribution build" OFF)
option(ENABLE_IMGUI "Include ImGui Dependency" ${PROJECT_IS_TOP_LEVEL})
option(ENABLE_SANDBOX "Build Sandbox demo project" ${PROJECT_IS_TOP_LEVEL})

if (ENGINE_DEBUG OR ENGINE_RELEASE)
    set(GLACIAL_ENGINE_ONLY ON CACHE BOOL "" FORCE)
    set(ENABLE_IMGUI OFF CACHE BOOL "" FORCE)
    set(ENABLE_SANDBOX OFF CACHE BOOL "" FORCE)
endif ()

if (ENGINE_DEBUG)
    add_compile_definitions(ENGINE_DEBUG)
elseif (ENGINE_RELEASE)
    add_compile_definitions(ENGINE_RELEASE)
endif ()

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