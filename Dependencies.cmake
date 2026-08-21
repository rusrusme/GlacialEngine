# Dependencies.cmake
# ----------------------------------------------------------------------------
include(FetchContent)
# ----------------------------------------------------------------------------
# Fetch GLFW
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.5.1
)
FetchContent_MakeAvailable(glfw)
# ----------------------------------------------------------------------------
# Fetch GLM
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.3
)
FetchContent_MakeAvailable(glm)
# ----------------------------------------------------------------------------
# Fetch EnTT
FetchContent_Declare(
        entt
        GIT_REPOSITORY https://github.com/skypjack/entt.git
        GIT_TAG v4.0.0
)
FetchContent_MakeAvailable(entt)
# ----------------------------------------------------------------------------
# Fetch FreeType (Dependency of RmlUi)
set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
FetchContent_Declare(
        freetype
        GIT_REPOSITORY https://github.com/freetype/freetype.git
        GIT_TAG VER-2-14-3
)
FetchContent_MakeAvailable(freetype)
if (TARGET freetype AND NOT TARGET Freetype::Freetype)
    add_library(Freetype::Freetype ALIAS freetype)
endif ()
# ----------------------------------------------------------------------------
# Fetch RmlUi
set(RMLUI_ENABLE_SAMPLES OFF CACHE BOOL "" FORCE)
set(RMLUI_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
        rmlui
        GIT_REPOSITORY https://github.com/mikke89/RmlUi.git
        GIT_TAG 6.2
)
FetchContent_MakeAvailable(rmlui)
# ----------------------------------------------------------------------------
# Fetch spdlog
if (ENABLE_SPDLOG)
    FetchContent_Declare(
            spdlog
            GIT_REPOSITORY https://github.com/gabime/spdlog.git
            GIT_TAG v1.17.0
    )
    FetchContent_MakeAvailable(spdlog)
endif ()
# ----------------------------------------------------------------------------
# Fetch ImGui
if (ENABLE_IMGUI)
    FetchContent_Declare(
            imgui
            GIT_REPOSITORY https://github.com/ocornut/imgui.git
            GIT_TAG v1.92.9b-docking
    )
    FetchContent_MakeAvailable(imgui)
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
endif ()
# ----------------------------------------------------------------------------
# Fetch ImGuizmo
if (ENABLE_IMGUI)
    FetchContent_Declare(
            imguizmo
            GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo.git
            GIT_TAG 1.10
    )
    FetchContent_MakeAvailable(imguizmo)
    target_link_libraries(imguizmo PUBLIC imgui)
endif ()
# ----------------------------------------------------------------------------
# Fetch cgltf
FetchContent_Declare(
        cgltf
        GIT_REPOSITORY https://github.com/jkuhlmann/cgltf.git
        GIT_TAG v1.15
)
FetchContent_MakeAvailable(cgltf)
if (NOT TARGET cgltf)
    add_library(cgltf INTERFACE)
    target_include_directories(cgltf INTERFACE ${cgltf_SOURCE_DIR})
endif ()
# ----------------------------------------------------------------------------
# Fetch json
FetchContent_Declare(
        json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)
# ----------------------------------------------------------------------------
