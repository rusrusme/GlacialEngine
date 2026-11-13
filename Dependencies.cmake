include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "" FORCE)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_FOLDER "ThirdParty")

# Helper function
function(fetch_dependency NAME TARGET ALIAS_NAME REPO TAG)
    FetchContent_Declare(
            ${NAME}
            GIT_REPOSITORY ${REPO}
            GIT_TAG ${TAG}
            GIT_SHALLOW TRUE
            GIT_SUBMODULES ""
    )
    FetchContent_MakeAvailable(${NAME})
    string(TOLOWER "${NAME}" NAME_LOWER)
    set(${NAME_LOWER}_SOURCE_DIR "${${NAME_LOWER}_SOURCE_DIR}" PARENT_SCOPE)
    set(${NAME_LOWER}_BINARY_DIR "${${NAME_LOWER}_BINARY_DIR}" PARENT_SCOPE)
    if (TARGET "${TARGET}" AND NOT "${ALIAS_NAME}" STREQUAL "" AND NOT TARGET "${ALIAS_NAME}")
        add_library(${ALIAS_NAME} ALIAS ${TARGET})
    endif ()
endfunction()

# Fetch GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
fetch_dependency(glfw glfw DEPENDENCY::GLFW https://github.com/glfw/glfw.git 3.5.1)
foreach (target glfw update_mappings)
    if (TARGET ${target})
        set_target_properties(${target} PROPERTIES FOLDER "ThirdParty")
    endif ()
endforeach ()

# Fetch SDL3
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
set(SDL_AUDIO OFF CACHE BOOL "" FORCE)
set(SDL_CAMERA OFF CACHE BOOL "" FORCE)
set(SDL_RENDER OFF CACHE BOOL "" FORCE)
set(SDL_GPU OFF CACHE BOOL "" FORCE)
fetch_dependency(SDL3 SDL3-static DEPENDENCY::SDL https://github.com/libsdl-org/SDL.git release-3.4.18)

# Fetch GLM
set(GLM_BUILD_LIBRARY OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_INSTALL OFF CACHE BOOL "" FORCE)
fetch_dependency(glm glm DEPENDENCY::GLM https://github.com/g-truc/glm.git 1.0.3)

# Fetch EnTT
set(ENTT_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(ENTT_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
set(ENTT_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(ENTT_INSTALL OFF CACHE BOOL "" FORCE)
fetch_dependency(entt EnTT DEPENDENCY::ENTT https://github.com/skypjack/entt.git v4.0.0)

# Fetch FreeType (Dependency of RmlUi)
set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
set(FT_ENABLE_ERROR_STRINGS OFF CACHE BOOL "" FORCE)
fetch_dependency(freetype freetype Freetype::Freetype https://github.com/freetype/freetype.git VER-2-14-3)

# Fetch RmlUi
set(RMLUI_SAMPLES OFF CACHE BOOL "" FORCE)
set(RMLUI_LUA_BINDINGS OFF CACHE BOOL "" FORCE)
set(RMLUI_LOTTIE_PLUGIN OFF CACHE BOOL "" FORCE)
set(RMLUI_SVG_PLUGIN OFF CACHE BOOL "" FORCE)
set(RMLUI_TRACY_PROFILING OFF CACHE BOOL "" FORCE)
set(RMLUI_INSTALL_RUNTIME_DEPENDENCIES OFF CACHE BOOL "" FORCE)
fetch_dependency(rmlui rmlui DEPENDENCY::RMLUI https://github.com/mikke89/RmlUi.git 6.2)

# Fetch ImGui
if (ENABLE_IMGUI)
    fetch_dependency(imgui "" "" https://github.com/ocornut/imgui.git v1.92.9b-docking)
    if (NOT TARGET imgui)
        add_library(imgui STATIC
                    ${imgui_SOURCE_DIR}/imgui.cpp
                    ${imgui_SOURCE_DIR}/imgui_draw.cpp
                    ${imgui_SOURCE_DIR}/imgui_tables.cpp
                    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
                    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
                    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        )
        target_include_directories(imgui PUBLIC
                                   ${imgui_SOURCE_DIR}
                                   ${imgui_SOURCE_DIR}/backends
        )
        target_link_libraries(imgui PUBLIC glfw)
    endif ()
    add_library(DEPENDENCY::IMGUI ALIAS imgui)
endif ()

# Fetch ImGuizmo
if (ENABLE_IMGUI)
    set(IMGUIZMO_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
    fetch_dependency(imguizmo imguizmo DEPENDENCY::IMGUIZMO https://github.com/CedricGuillemet/ImGuizmo.git 1.10)
    target_link_libraries(imguizmo PUBLIC imgui)
endif ()

# Fetch cgltf
fetch_dependency(cgltf "" "" https://github.com/jkuhlmann/cgltf.git v1.15)
if (NOT TARGET cgltf)
    add_library(cgltf INTERFACE)
    target_include_directories(cgltf INTERFACE ${cgltf_SOURCE_DIR})
endif ()
add_library(DEPENDENCY::CGLTF ALIAS cgltf)

# Fetch json
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
set(JSON_Install OFF CACHE BOOL "" FORCE)
fetch_dependency(json nlohmann_json DEPENDENCY::NLOHMANN https://github.com/nlohmann/json.git v3.11.3)

# Fetch miniaudio
set(MINIAUDIO_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(MINIAUDIO_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(MINIAUDIO_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(MINIAUDIO_INSTALL OFF CACHE BOOL "" FORCE)
fetch_dependency(miniaudio miniaudio DEPENDENCY::MINIAUDIO https://github.com/mackron/miniaudio.git 0.11.25)

add_library(glad STATIC lib/glad/src/glad.c)
target_include_directories(glad PUBLIC lib/glad/include)
add_library(DEPENDENCY::GLAD ALIAS glad)

add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE lib/stb_image)
add_library(DEPENDENCY::STB_IMAGE ALIAS stb_image)

unset(CMAKE_FOLDER)