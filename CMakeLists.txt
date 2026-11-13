# CMakeLists.txt
# ----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.20)
# ----------------------------------------------------------------------------
project(GlacialEngine CXX C)
set(CMAKE_CXX_STANDARD 20)
# ----------------------------------------------------------------------------
option(ENGINE_DEBUG "Engine-only debug distribution build" OFF)
option(ENGINE_RELEASE "Engine-only release distribution build" OFF)
option(ENABLE_IMGUI "Include ImGui Dependency" ON)
option(ENABLE_SANDBOX "Build Sandbox demo project" ON)

if (ENGINE_DEBUG OR ENGINE_RELEASE)
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
# ----------------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# ----------------------------------------------------------------------------
include(Dependencies.cmake)
# ----------------------------------------------------------------------------
set(CMAKE_FOLDER "GlacialEngine")
add_subdirectory(GlazeScript)
unset(CMAKE_FOLDER)

add_subdirectory(engine)
if (NOT ENGINE_DEBUG AND NOT ENGINE_RELEASE)
    add_subdirectory(editor)

    set(CMAKE_FOLDER "Tests")
    add_subdirectory(test)
    unset(CMAKE_FOLDER)

    if (ENABLE_SANDBOX AND NOT ENGINE_DEBUG AND NOT ENGINE_RELEASE)
        add_subdirectory(sandbox)
    endif ()
endif ()
# ----------------------------------------------------------------------------


