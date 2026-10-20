# engine-lib/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(GlacialEngineLib LANGUAGES CXX C)

# Define core interface library target
add_library(Engine INTERFACE)
add_library(GlacialEngine::Engine ALIAS Engine)

# Force C++20 requirement for consumers
target_compile_features(Engine INTERFACE cxx_std_20)

# Include search path for engine headers
target_include_directories(Engine INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Find system dependencies
find_package(OpenGL REQUIRED)
find_package(Vulkan REQUIRED)

# Resolve Shaderc
if(TARGET Vulkan::shaderc_combined)
    add_library(Vulkan::Shaderc ALIAS Vulkan::shaderc_combined)
elseif(TARGET Vulkan::shaderc_shared)
    add_library(Vulkan::Shaderc ALIAS Vulkan::shaderc_shared)
else()
    find_library(SHADERC_LIB
                 NAMES shaderc_shared shaderc_combined
                 HINTS $ENV{VULKAN_SDK}/Lib
    )
    add_library(Vulkan::Shaderc UNKNOWN IMPORTED)
    set_target_properties(Vulkan::Shaderc PROPERTIES
                          IMPORTED_LOCATION "${SHADERC_LIB}"
                          INTERFACE_INCLUDE_DIRECTORIES "${Vulkan_INCLUDE_DIRS}"
    )
endif()

# Render API Selection
set(RENDER_API "OPENGL" CACHE STRING "Selects the rendering backend")
set_property(CACHE RENDER_API PROPERTY STRINGS "OPENGL" "VULKAN" "DX12")

if (RENDER_API STREQUAL "OPENGL")
    target_compile_definitions(Engine INTERFACE RENDER_API_OPENGL)
elseif (RENDER_API STREQUAL "VULKAN")
    target_compile_definitions(Engine INTERFACE RENDER_API_VULKAN)
elseif (RENDER_API STREQUAL "DX12")
    target_compile_definitions(Engine INTERFACE RENDER_API_DX12)
endif ()

# Preprocessor definitions
target_compile_definitions(Engine INTERFACE
                           GLFW_INCLUDE_NONE
                           $<$<CONFIG:Debug>:DEBUG_DIST>
                           $<$<NOT:$<CONFIG:Debug>>:RELEASE_DIST>
)

# Glob precompiled libraries for Windows (.lib) and Linux (.a)
file(GLOB_RECURSE ENGINE_DEBUG_LIBS CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/lib/debug/*.lib"
     "${CMAKE_CURRENT_SOURCE_DIR}/lib/debug/*.a"
)
file(GLOB_RECURSE ENGINE_RELEASE_LIBS CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/lib/release/*.lib"
     "${CMAKE_CURRENT_SOURCE_DIR}/lib/release/*.a"
)

target_link_libraries(Engine INTERFACE
                      $<$<CONFIG:Debug>:${ENGINE_DEBUG_LIBS}>
                      $<$<NOT:$<CONFIG:Debug>>:${ENGINE_RELEASE_LIBS}>
                      OpenGL::GL
                      Vulkan::Shaderc
)

# Helper function to auto-copy engine and user assets to target binary directory
function(link_assets TARGET_NAME)
    set(ENGINE_ASSETS_DIR "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/assets")
    set(USER_ASSETS_DIR "${ARGN}")

    if (EXISTS "${ENGINE_ASSETS_DIR}/fonts")
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_directory
                           "${ENGINE_ASSETS_DIR}/fonts"
                           "$<TARGET_FILE_DIR:${TARGET_NAME}>/assets/fonts"
                           COMMENT "Copying Engine fonts to output folder..."
        )
    endif ()

    if (EXISTS "${ENGINE_ASSETS_DIR}/shaders")
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_directory
                           "${ENGINE_ASSETS_DIR}/shaders"
                           "$<TARGET_FILE_DIR:${TARGET_NAME}>/assets/shaders"
                           COMMENT "Copying Engine shaders to output folder..."
        )
    endif ()

    if (USER_ASSETS_DIR AND EXISTS "${USER_ASSETS_DIR}")
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_directory
                           "${USER_ASSETS_DIR}"
                           "$<TARGET_FILE_DIR:${TARGET_NAME}>/assets"
                           COMMENT "Copying User game assets to output folder..."
        )
    endif ()
endfunction()