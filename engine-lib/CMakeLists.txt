# engine-lib/CMakeLists.txt
cmake_minimum_required(VERSION 4.3)
project(EngineLib LANGUAGES CXX C)

add_library(Engine INTERFACE)
add_library(Engine::Engine ALIAS Engine)

target_compile_features(Engine INTERFACE cxx_std_20)

target_include_directories(Engine INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_compile_definitions(Engine INTERFACE
                           GLFW_INCLUDE_NONE
                           $<$<CONFIG:Debug>:DEBUG_DIST>
                           $<$<NOT:$<CONFIG:Debug>>:RELEASE_DIST>
)

find_package(OpenGL REQUIRED)

file(GLOB ENGINE_DEBUG_LIBS "${CMAKE_CURRENT_SOURCE_DIR}/lib/debug/*.lib")
file(GLOB ENGINE_RELEASE_LIBS "${CMAKE_CURRENT_SOURCE_DIR}/lib/release/*.lib")

target_link_libraries(Engine INTERFACE
                      $<$<CONFIG:Debug>:${ENGINE_DEBUG_LIBS}>
                      $<$<NOT:$<CONFIG:Debug>>:${ENGINE_RELEASE_LIBS}>
                      OpenGL::GL
)

function(link_assets TARGET_NAME)
    set(ENGINE_ASSETS_DIR "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/assets")
    set(USER_ASSETS_DIR "${ARGN}")

    if (EXISTS "${ENGINE_ASSETS_DIR}")
        file(GLOB_RECURSE ENGINE_ASSETS CONFIGURE_DEPENDS "${ENGINE_ASSETS_DIR}/*")
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_directory
                           "${ENGINE_ASSETS_DIR}"
                           "$<TARGET_FILE_DIR:${TARGET_NAME}>/assets"
                           COMMENT "Copying Engine core assets..."
        )
    endif ()

    # Allow custom assets
    if (USER_ASSETS_DIR AND EXISTS "${USER_ASSETS_DIR}")
        file(GLOB_RECURSE USER_ASSETS CONFIGURE_DEPENDS "${USER_ASSETS_DIR}/*")
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_directory
                           "${USER_ASSETS_DIR}"
                           "$<TARGET_FILE_DIR:${TARGET_NAME}>/assets"
                           COMMENT "Copying User game assets..."
        )
    endif ()
endfunction()