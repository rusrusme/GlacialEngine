# engine-lib/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(BasisEngineLib LANGUAGES CXX C)

# Define core interface library target
add_library(Engine INTERFACE)
add_library(BasisEngine::Engine ALIAS Engine)

# Force C++20 requirement for consumers
target_compile_features(Engine INTERFACE cxx_std_20)

# Include search path for engine headers
target_include_directories(Engine INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Preprocessor definitions
target_compile_definitions(Engine INTERFACE
                           GLFW_INCLUDE_NONE
                           $<$<CONFIG:Debug>:DEBUG_DIST>
                           $<$<NOT:$<CONFIG:Debug>>:RELEASE_DIST>
)

# Find system OpenGL
find_package(OpenGL REQUIRED)

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
)

# Helper function to auto-copy engine and user assets to target binary directory
function(link_assets TARGET_NAME)
    set(ENGINE_ASSETS_DIR "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/assets")
    set(USER_ASSETS_DIR "${ARGN}")

    if (EXISTS "${ENGINE_ASSETS_DIR}")
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_directory
                           "${ENGINE_ASSETS_DIR}"
                           "$<TARGET_FILE_DIR:${TARGET_NAME}>/assets"
                           COMMENT "Copying Engine core assets to output folder..."
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