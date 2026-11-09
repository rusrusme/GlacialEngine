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
    set(GSHADER_BACKEND "opengl")
    find_package(OpenGL REQUIRED)
    target_link_libraries(Engine PUBLIC OpenGL::GL DEPENDENCY::GLAD)
# ----------------------------------------------------------------------------
elseif (RENDER_API STREQUAL "VULKAN")
    target_compile_definitions(Engine PUBLIC RENDER_API_VULKAN)
    set(GSHADER_BACKEND "vulkan")
    find_package(Vulkan REQUIRED)
# ----------------------------------------------------------------------------
elseif (RENDER_API STREQUAL "DX12")
    if (NOT WIN32)
        message(FATAL_ERROR "DirectX 12 is only supported on Windows platforms")
    endif ()
    target_compile_definitions(Engine PUBLIC RENDER_API_DX12)
    target_link_libraries(Engine PUBLIC d3d12.lib dxgi.lib dxguid.lib)
    set(GSHADER_BACKEND "dx12")
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
    target_compile_definitions(Engine PUBLIC GLFW_INCLUDE_NONE)
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
                      DEPENDENCY::GLACIALC

                      INTERNAL::GLAZESCRIPT
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
file(GLOB_RECURSE SHADER_SOURCES RELATIVE "${SOURCE_ASSETS}/shaders" CONFIGURE_DEPENDS "${SOURCE_ASSETS}/shaders/*.gshader")
set(COMPILED_SHADERS "")

foreach(SHADER_REL IN LISTS SHADER_SOURCES)
    # Input
    set(SHADER_IN "${SOURCE_ASSETS}/shaders/${SHADER_REL}")
    # Output
    cmake_path(REPLACE_EXTENSION SHADER_REL "gsb" OUTPUT_VARIABLE SHADER_OUT_REL)
    set(SHADER_OUT "${RUNTIME_ASSETS}/shaders/compiled/${SHADER_OUT_REL}")

    add_custom_command(
            OUTPUT "${SHADER_OUT}"
            COMMAND $<TARGET_FILE:glacialc::glacialc> "${SHADER_IN}" -o "${SHADER_OUT}" -b ${GSHADER_BACKEND}
            DEPENDS "${SHADER_IN}" $<TARGET_FILE:glacialc::glacialc>
            COMMENT "Compiling shader: ${SHADER_REL} [${GSHADER_BACKEND}]"
            VERBATIM
    )
    list(APPEND COMPILED_SHADERS "${SHADER_OUT}")
endforeach()
add_custom_target(CompileShaders ALL DEPENDS ${COMPILED_SHADERS})
# ----------------------------------------------------------------------------
# Generate cmd file template
if (WIN32)
    set(SCRIPT_NAME "BuildScripts.cmd")
    set(SCRIPT_PERMS "")
else ()
    set(SCRIPT_NAME "BuildScripts.sh")
    set(SCRIPT_PERMS
        FILE_PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
    )
endif ()

configure_file(
        "${CMAKE_SOURCE_DIR}/${SCRIPT_NAME}.in"
        "${CMAKE_CURRENT_BINARY_DIR}/${SCRIPT_NAME}"
        @ONLY
        ${SCRIPT_PERMS}
)

add_custom_target(SetupBuildScripts ALL
                  COMMAND ${CMAKE_COMMAND} -E copy_if_different
                  "${CMAKE_CURRENT_BINARY_DIR}/${SCRIPT_NAME}"
                  "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${SCRIPT_NAME}"
                  COMMENT "Syncing ${SCRIPT_NAME} to runtime directory"
                  VERBATIM
)
# ----------------------------------------------------------------------------
set(SCRIPT_OUT_DIR "${RUNTIME_ASSETS}/scripts/compiled")
set(SCRIPT_BUNDLE_CPP "${SCRIPT_OUT_DIR}/GlazeScriptsBundle.cpp")

file(GLOB_RECURSE GSCRIPT_SOURCES RELATIVE "${SOURCE_ASSETS}/scripts" CONFIGURE_DEPENDS "${SOURCE_ASSETS}/scripts/*.gscript")

set(COMPILED_SCRIPTS "")
foreach(SCRIPT_REL IN LISTS GSCRIPT_SOURCES)
    set(SCRIPT_IN "${SOURCE_ASSETS}/scripts/${SCRIPT_REL}")
    cmake_path(REPLACE_EXTENSION SCRIPT_REL "hpp" OUTPUT_VARIABLE SCRIPT_OUT_REL)
    set(SCRIPT_OUT "${SCRIPT_OUT_DIR}/${SCRIPT_OUT_REL}")

    add_custom_command(
            OUTPUT "${SCRIPT_OUT}"
            COMMAND $<TARGET_FILE:glazec> "${SCRIPT_IN}" -o "${SCRIPT_OUT_DIR}" -e "${CMAKE_SOURCE_DIR}"
            DEPENDS "${SCRIPT_IN}" $<TARGET_FILE:glazec>
            COMMENT "Transpiling GlazeScript: ${SCRIPT_REL}"
            VERBATIM
    )
    list(APPEND COMPILED_SCRIPTS "${SCRIPT_OUT}")
endforeach()

if (COMPILED_SCRIPTS)
    add_custom_command(
            OUTPUT "${SCRIPT_BUNDLE_CPP}"
            COMMAND ${CMAKE_COMMAND} -E touch_nocreate "${SCRIPT_BUNDLE_CPP}"
            DEPENDS ${COMPILED_SCRIPTS}
            VERBATIM
    )

    add_library(Scripts SHARED "${SCRIPT_BUNDLE_CPP}")

    target_precompile_headers(Scripts REUSE_FROM Engine)

    target_include_directories(Scripts PRIVATE
                               "${SCRIPT_OUT_DIR}"
                               "${CMAKE_SOURCE_DIR}/GlazeScript/include"
    )

    target_link_libraries(Scripts PRIVATE Engine)

    set_target_properties(Scripts PROPERTIES
                          RUNTIME_OUTPUT_DIRECTORY "${RUNTIME_ASSETS}/scripts"
                          LIBRARY_OUTPUT_DIRECTORY "${RUNTIME_ASSETS}/scripts"
                          PREFIX ""
                          FOLDER "GlacialEngine"
    )
    target_include_directories(Engine PUBLIC "${SCRIPT_OUT_DIR}")
endif()
# ----------------------------------------------------------------------------
add_dependencies(Engine SyncAssets)
add_dependencies(Engine CompileShaders)
add_dependencies(Engine SetupBuildScripts)
set_target_properties(SyncAssets CompileShaders SetupBuildScripts PROPERTIES FOLDER "GlacialEngine")
# ----------------------------------------------------------------------------