file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
     "src/*.cpp"
     "src/*.hpp"
     "src/*.h"
     "include/*.cpp"
     "include/*.hpp"
     "include/*.h"
)
add_library(Engine STATIC ${ENGINE_SOURCES})
add_library(Glacial::Engine ALIAS Engine)

target_include_directories(Engine
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

include(GenerateUmbrella.cmake)

if (WIN32)
    target_compile_definitions(Engine PUBLIC NOMINMAX WIN32_LEAN_AND_MEAN)
    target_link_libraries(Engine PUBLIC comdlg32.lib)
endif ()

set(RENDER_API "OPENGL" CACHE STRING "Selects the rendering backend")
set_property(CACHE RENDER_API PROPERTY STRINGS "OPENGL" "VULKAN" "DX12")

if (RENDER_API STREQUAL "OPENGL")
    target_compile_definitions(Engine PUBLIC RENDER_API_OPENGL)
    set(GSHADER_BACKEND "opengl")
    find_package(OpenGL REQUIRED)
    target_link_libraries(Engine PUBLIC OpenGL::GL DEPENDENCY::GLAD)

elseif (RENDER_API STREQUAL "VULKAN")
    target_compile_definitions(Engine PUBLIC RENDER_API_VULKAN)
    set(GSHADER_BACKEND "vulkan")
    find_package(Vulkan REQUIRED)

elseif (RENDER_API STREQUAL "DX12")
    if (NOT WIN32)
        message(FATAL_ERROR "DirectX 12 is only supported on Windows platforms")
    endif ()
    target_compile_definitions(Engine PUBLIC RENDER_API_DX12)
    target_link_libraries(Engine PUBLIC d3d12.lib dxgi.lib dxguid.lib)
    set(GSHADER_BACKEND "dx12")

else ()
    message(FATAL_ERROR "Unsupported Render API: ${RENDER_API}")
endif ()

set(WINDOW_BACKEND "GLFW" CACHE STRING "Windowing backend (GLFW or SDL)")
set_property(CACHE WINDOW_BACKEND PROPERTY STRINGS "GLFW" "SDL")

if (WINDOW_BACKEND STREQUAL "GLFW")
    target_compile_definitions(Engine PUBLIC WINDOW_BACKEND_GLFW)
    target_compile_definitions(Engine PUBLIC GLFW_INCLUDE_NONE)
    target_link_libraries(Engine PUBLIC DEPENDENCY::GLFW)
elseif (WINDOW_BACKEND STREQUAL "SDL")
    target_compile_definitions(Engine PUBLIC WINDOW_BACKEND_SDL)
    target_link_libraries(Engine PUBLIC DEPENDENCY::SDL)
else ()
    message(FATAL_ERROR "Unsupported Window Backend: ${WINDOW_BACKEND}")
endif ()

target_link_libraries(Engine PUBLIC
                      DEPENDENCY::RMLUI
                      DEPENDENCY::STB_IMAGE
                      DEPENDENCY::GLM
                      DEPENDENCY::ENTT
                      DEPENDENCY::CGLTF
                      DEPENDENCY::MINIAUDIO
                      DEPENDENCY::NLOHMANN

                      INTERNAL::GLAZESCRIPT
                      INTERNAL::GSHADER
)

if (ENABLE_IMGUI)
    target_compile_definitions(Engine PUBLIC ENABLE_IMGUI)
    target_link_libraries(Engine PUBLIC DEPENDENCY::IMGUI)
endif ()

target_precompile_headers(Engine PRIVATE src/core/core_pch.hpp)

set(SOURCE_ASSETS "${PROJECT_SOURCE_DIR}/assets")
set(RUNTIME_ASSETS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets")

include(SyncAssets.cmake)
include(CompileShaders.cmake)
include(BuildScripts.cmake)