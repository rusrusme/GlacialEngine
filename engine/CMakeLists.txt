# engine/CMakeLists.txt
# ----------------------------------------------------------------------------
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
     "src/*.cpp"
     "src/*.hpp"
     "src/*.h"
     "include/*.cpp"
     "include/*.hpp"
     "include/*.h"
)
add_library(Engine STATIC ${ENGINE_SOURCES})
# ----------------------------------------------------------------------------
target_include_directories(Engine
                           PUBLIC ${CMAKE_SOURCE_DIR}/engine/include
                           PRIVATE ${CMAKE_SOURCE_DIR}/engine/src
)
# ----------------------------------------------------------------------------
if (WIN32)
    target_compile_definitions(Engine PUBLIC NOMINMAX WIN32_LEAN_AND_MEAN)
    target_link_libraries(Engine PUBLIC comdlg32.lib)
endif ()
# ----------------------------------------------------------------------------
set(RENDER_API "OPENGL" CACHE STRING "Selects the rendering backend")
set_property(CACHE RENDER_API PROPERTY STRINGS "OPENGL" "VULKAN" "DX12")
# ----------------------------------------------------------------------------
if (RENDER_API STREQUAL "OPENGL")
    target_compile_definitions(Engine PUBLIC RENDER_API_OPENGL)
    set(BSHC_BACKEND "opengl")
    find_package(OpenGL REQUIRED)
    target_link_libraries(Engine PUBLIC OpenGL::GL DEPENDENCY::GLAD)
# ----------------------------------------------------------------------------
elseif (RENDER_API STREQUAL "VULKAN")
    target_compile_definitions(Engine PUBLIC RENDER_API_VULKAN)
    set(BSHC_BACKEND "vulkan")
    find_package(Vulkan REQUIRED)
# ----------------------------------------------------------------------------
elseif (RENDER_API STREQUAL "DX12")
    if (NOT WIN32)
        message(FATAL_ERROR "DirectX 12 is only supported on Windows platforms")
    endif ()
    target_compile_definitions(Engine PUBLIC RENDER_API_DX12)
    target_link_libraries(Engine PUBLIC d3d12.lib dxgi.lib dxguid.lib)
    set(BSHC_BACKEND "dx12")
# ----------------------------------------------------------------------------
else ()
    message(FATAL_ERROR "Unsupported Render API: ${RENDER_API}")
endif ()
# ----------------------------------------------------------------------------
set(WINDOW_BACKEND "GLFW" CACHE STRING "Windowing backend (GLFW or SDL)")
set_property(CACHE WINDOW_BACKEND PROPERTY STRINGS "GLFW" "SDL")
# ----------------------------------------------------------------------------
if (WINDOW_BACKEND STREQUAL "GLFW")
    target_compile_definitions(Engine PUBLIC WINDOW_BACKEND_GLFW)
    target_compile_definitions(Engine PRIVATE GLFW_INCLUDE_NONE)
    target_link_libraries(Engine PUBLIC DEPENDENCY::GLFW)
elseif (WINDOW_BACKEND STREQUAL "SDL")
    target_compile_definitions(Engine PUBLIC WINDOW_BACKEND_SDL)
    target_link_libraries(Engine PUBLIC DEPENDENCY::SDL)
else ()
    message(FATAL_ERROR "Unsupported Window Backend: ${WINDOW_BACKEND}")
endif ()
# ----------------------------------------------------------------------------
target_link_libraries(Engine PUBLIC
                      DEPENDENCY::RMLUI
                      DEPENDENCY::STB_IMAGE
                      DEPENDENCY::GLM
                      DEPENDENCY::ENTT
                      DEPENDENCY::CGLTF
                      DEPENDENCY::MINIAUDIO
                      DEPENDENCY::NLOHMANN
                      DEPENDENCY::BSHC
)
# ----------------------------------------------------------------------------
if (ENABLE_IMGUI)
    target_link_libraries(Engine PUBLIC DEPENDENCY::IMGUI)
endif ()
# ----------------------------------------------------------------------------
target_precompile_headers(Engine PRIVATE src/core/core_pch.hpp)
# ----------------------------------------------------------------------------
include(PackageDistribution.cmake)
# ----------------------------------------------------------------------------
set(RUNTIME_ASSETS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets")
set(SOURCE_ASSETS "${CMAKE_SOURCE_DIR}/assets")
add_custom_target(SyncAssets ALL
                  COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${SOURCE_ASSETS}" "${RUNTIME_ASSETS}"
                  COMMENT "Syncing assets to runtime output directory"
                  VERBATIM
)
# ----------------------------------------------------------------------------
file(GLOB_RECURSE BSH_SOURCES RELATIVE "${SOURCE_ASSETS}/shaders" CONFIGURE_DEPENDS "${SOURCE_ASSETS}/shaders/*.bsh")
set(COMPILED_SHADERS "")

foreach(SHADER_REL IN LISTS BSH_SOURCES)
    set(SHADER_IN "${SOURCE_ASSETS}/shaders/${SHADER_REL}")
    set(SHADER_OUT "${RUNTIME_ASSETS}/shaders/compiled/${SHADER_REL}.bin")
    add_custom_command(
            OUTPUT "${SHADER_OUT}"
            COMMAND $<TARGET_FILE:bshc::bshc> "${SHADER_IN}" -o "${SHADER_OUT}" -b ${BSHC_BACKEND}
            DEPENDS "${SHADER_IN}" $<TARGET_FILE:bshc::bshc>
            COMMENT "Compiling shader: ${SHADER_REL} [${BSHC_BACKEND}]"
            VERBATIM
    )
    list(APPEND COMPILED_SHADERS "${SHADER_OUT}")
endforeach()
add_custom_target(CompileShaders ALL DEPENDS ${COMPILED_SHADERS})
# ----------------------------------------------------------------------------
add_dependencies(Engine SyncAssets)
add_dependencies(Engine CompileShaders)
# ----------------------------------------------------------------------------