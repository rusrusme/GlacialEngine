# CMakeLists.txt
# ----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.20)
# ----------------------------------------------------------------------------
project(BasisEngine CXX C)
set(CMAKE_CXX_STANDARD 20)
# ----------------------------------------------------------------------------
option(ENGINE_DEBUG_DIST "Engine-only debug distribution build for developers" OFF)
option(ENGINE_RELEASE_DIST "Engine-only release distribution build" OFF)
option(ENABLE_IMGUI "Include ImGui Dependency" ON)
option(BUILD_SANDBOX "Build Sandbox demo project" ON)

if (ENGINE_DEBUG_DIST)
    set(ENGINE_DEBUG_DIST ON CACHE BOOL "" FORCE)
    add_compile_definitions(ENGINE_DEBUG_DIST)
elseif (ENGINE_RELEASE_DIST)
    set(ENGINE_RELEASE_DIST ON CACHE BOOL "" FORCE)
    add_compile_definitions(ENGINE_RELEASE_DIST)
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
add_subdirectory(Engine)
if (NOT ENGINE_DEBUG_DIST AND NOT ENGINE_RELEASE_DIST)
    # TEMPORARY add_subdirectory(Editor)
    add_subdirectory(test)
    add_subdirectory(Sandbox)
endif ()
# ----------------------------------------------------------------------------


