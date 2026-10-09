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
# ----------------------------------------------------------------------------
set_property(CACHE RENDER_API PROPERTY STRINGS "OPENGL" "VULKAN" "DX12")
if (RENDER_API STREQUAL "OPENGL")
    target_compile_definitions(Engine PUBLIC RENDER_API_OPENGL)
    set(BSHC_BACKEND "opengl")
elseif (RENDER_API STREQUAL "VULKAN")
    target_compile_definitions(Engine PUBLIC RENDER_API_VULKAN)
    set(BSHC_BACKEND "vulkan")
elseif (RENDER_API STREQUAL "DX12")
    if (NOT WIN32)
        message(FATAL_ERROR "DirectX 12 is only supported on Windows platforms")
    endif ()
    target_compile_definitions(Engine PUBLIC RENDER_API_DX12)
    target_link_libraries(Engine PUBLIC d3d12.lib dxgi.lib dxguid.lib)
    set(BSHC_BACKEND "dx12")
else ()
    message(FATAL_ERROR "Unsupported Render API: ${RENDER_API}")
endif ()
# ----------------------------------------------------------------------------
find_package(OpenGL REQUIRED)
target_link_libraries(Engine PUBLIC
                      glad
                      glfw
                      rmlui
                      stb_image
                      glm::glm
                      EnTT::EnTT
                      cgltf
                      OpenGL::GL
                      miniaudio
                      nlohmann_json::nlohmann_json
                      bshc::bshc_core
)
# ----------------------------------------------------------------------------
if (ENABLE_IMGUI)
    target_link_libraries(Engine PUBLIC imgui)
endif ()
# ----------------------------------------------------------------------------
target_compile_definitions(Engine PRIVATE GLFW_INCLUDE_NONE)
# ----------------------------------------------------------------------------
target_precompile_headers(Engine PRIVATE src/core/core_pch.hpp)
# ----------------------------------------------------------------------------
include(PackageDistribution.cmake)
# ----------------------------------------------------------------------------
set(RUNTIME_ASSETS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets")
set(SOURCE_ASSETS "${CMAKE_SOURCE_DIR}/assets")

add_custom_target(SyncAssets ALL
                  COMMAND ${CMAKE_COMMAND} -E remove_directory "${RUNTIME_ASSETS}"
                  COMMAND ${CMAKE_COMMAND} -E copy_directory "${SOURCE_ASSETS}" "${RUNTIME_ASSETS}"
                  COMMENT "Syncing assets to runtime output directory"
                  VERBATIM
)
# ----------------------------------------------------------------------------
file(GLOB_RECURSE BSH_SOURCES CONFIGURE_DEPENDS "${SOURCE_ASSETS}/*.bsh")

if (BSH_SOURCES)
    add_custom_target(CompileBSH ALL
                      COMMAND $<TARGET_FILE:bshc::bshc> ${BSH_SOURCES} -o "${RUNTIME_ASSETS}/shaders/compiled" -b ${BSHC_BACKEND}
                      COMMENT "Compiling shaders via BSHC (${BSHC_BACKEND}) into runtime directory"
                      VERBATIM
    )
else ()
    add_custom_target(CompileBSH ALL
                      COMMENT "No BSH shader sources found"
    )
endif ()
# ----------------------------------------------------------------------------
add_dependencies(Engine SyncAssets)
add_dependencies(Engine CompileBSH)
# ----------------------------------------------------------------------------