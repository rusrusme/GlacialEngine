# engine/CMakeLists.txt
# ----------------------------------------------------------------------------
add_library(Engine STATIC
            src/core/Application.cpp
            include/core/Application.hpp
            src/render/Renderer.cpp
            include/render/Renderer.hpp
            src/render/debug/DebugFrame.cpp
            include/render/debug/DebugFrame.hpp
            src/math/Raycaster.cpp
            include/math/Raycaster.hpp
            src/render/Camera.cpp
            include/render/Camera.hpp
            include/math/Ray.hpp
            include/render/mesh/Mesh.hpp
            include/render/Vertex.hpp
            src/input/CameraController.cpp
            include/input/CameraController.hpp
            include/input/Input.hpp
            include/core/Config.hpp
            src/core/Log.cpp
            include/core/Log.hpp
            include/render/Frame.hpp
            src/render/Frame.cpp
            src/scene/Scene.cpp
            include/scene/Scene.hpp
            src/scene/ecs/Entity.cpp
            include/scene/ecs/Entity.hpp
            include/core/Layer.hpp
            include/scene/ecs/Components.hpp
            src/core/UUID.cpp
            include/core/UUID.hpp
            include/core/Engine.hpp
            src/core/LayerStack.cpp
            include/core/LayerStack.hpp
            src/scene/system/CameraSystem.cpp
            include/scene/system/CameraSystem.hpp
            include/input/KeyCodes.hpp
            src/core/Window.cpp
            include/core/Window.hpp
            include/event/Event.hpp
            include/event/WindowEvents.hpp
            include/event/MouseEvents.hpp
            include/render/Buffer.hpp
            src/render/BufferLayout.cpp
            include/render/BufferLayout.hpp
            include/render/VertexArray.hpp
            include/render/Shader.hpp
            include/render/UniformBuffer.hpp
            include/util/FileIO.hpp
            include/event/KeyEvents.hpp
            src/input/Input.cpp
            include/input/Input.hpp
            include/input/MouseCodes.hpp
            src/render/ShaderLibrary.cpp
            include/render/ShaderLibrary.hpp
            include/render/mesh/PrimitiveMesh.hpp
            src/scene/SceneSerializer.cpp
            include/scene/SceneSerializer.hpp
            src/render/mesh/MeshLoader.cpp
            include/render/mesh/MeshLoader.hpp
            src/render/mesh/Mesh.cpp
            include/render/mesh/Mesh.hpp
            include/asset/AssetType.hpp
            src/asset/MetafileHandler.cpp
            include/asset/MetafileHandler.hpp
            include/asset/AssetManager.hpp
            src/asset/AssetManager.cpp
            include/physics/AABB.hpp
            include/physics/SpatialHashGrid.hpp
            src/physics/SpatialHashGrid.cpp
            include/physics/CollisionSystem.hpp
            src/physics/CollisionSystem.cpp
            include/render/mesh/InstanceData.hpp
            src/ui/RmlRenderInterface.cpp
            src/ui/RmlRenderInterface.hpp
            src/ui/UISystem.cpp
            src/ui/UISystem.hpp
            src/ui/RmlSystemInterface.cpp
            src/ui/RmlSystemInterface.hpp
            src/ui/UILayer.cpp
            include/ui/UILayer.hpp
            src/ui/UIManager.cpp
            include/ui/UIManager.hpp
            include/render/Framebuffer.hpp
            src/physics/AABB.cpp
            include/physics/AABB.hpp
            src/render/mesh/PrimitiveMesh.cpp
            include/render/mesh/PrimitiveMesh.hpp
            include/render/Texture2D.hpp
            src/util/stb_image.cpp
            src/render/Frustum.cpp
            include/render/Frustum.hpp
            src/render/PostProcessor.cpp
            include/render/PostProcessor.hpp
            src/render/platform/opengl/GlFramebuffer.cpp
            include/render/platform/opengl/GlFramebuffer.hpp
            include/render/RenderAPI.hpp
            src/render/platform/opengl/GLRendererAPI.cpp
            include/render/platform/opengl/GLRendererAPI.hpp
            src/render/platform/opengl/GLShader.cpp
            include/render/platform/opengl/GLShader.hpp
            src/render/platform/opengl/GLTexture2D.cpp
            include/render/platform/opengl/GLTexture2D.hpp
            src/render/platform/opengl/GLUniformBuffer.cpp
            include/render/platform/opengl/GLUniformBuffer.hpp
            src/render/platform/opengl/GLVertexArray.cpp
            include/render/platform/opengl/GLVertexArray.hpp
            src/render/platform/opengl/GLBuffer.cpp
            include/render/platform/opengl/GLBuffer.hpp
            include/render/RenderCommand.hpp
            src/audio/AudioSystem.cpp
            include/audio/AudioSystem.hpp
            src/render/debug/DebugRenderer.cpp
            include/render/debug/DebugRenderer.hpp
            include/scene/SceneState.hpp
            include/scene/ecs/ComponentRegistry.hpp
            src/platform/FileDialogAPI.cpp
            include/platform/FileDialogAPI.hpp
            src/platform/win32/Win32FileDialog.cpp
            include/platform/win32/Win32FileDialog.hpp
            src/core/WindowAPI.cpp
            include/core/WindowAPI.hpp
            src/render/debug/ImguiPlatform.cpp
            include/render/debug/ImguiPlatform.hpp
            src/scene/SimulationState.cpp
            include/scene/SimulationState.hpp
            include/core/SystemContext.hpp
            src/scene/SceneManager.cpp
            include/scene/SceneManager.hpp
            src/render/RenderScope.cpp
            include/render/RenderScope.hpp
            src/platform/ShellApi.cpp
            include/platform/ShellApi.hpp
            src/platform/win32/Win32Shell.cpp
            include/platform/win32/Win32Shell.hpp
            src/render/BSHLoader.cpp
            include/render/BSHLoader.hpp
)
# ----------------------------------------------------------------------------
target_include_directories(Engine
                           PUBLIC ${CMAKE_SOURCE_DIR}/engine/include
                           PRIVATE ${CMAKE_SOURCE_DIR}/engine/src
)
# ----------------------------------------------------------------------------
if (WIN32)
    target_compile_definitions(Engine PUBLIC NOMINMAX WIN32_LEAN_AND_MEAN)
    target_link_libraries(Engine PUBLIC comdlg32.lib)
endif()
# ----------------------------------------------------------------------------
set(RENDER_API "OPENGL" CACHE STRING "Selects the rendering backend")
# ----------------------------------------------------------------------------
set_property(CACHE RENDER_API PROPERTY STRINGS "OPENGL" "VULKAN" "DX12")
if (RENDER_API STREQUAL "OPENGL")
    target_compile_definitions(Engine PUBLIC RENDER_API_OPENGL)
elseif (RENDER_API STREQUAL "VULKAN")
    target_compile_definitions(Engine PUBLIC RENDER_API_VULKAN)
elseif (RENDER_API STREQUAL "DX12")
    if (NOT WIN32)
        message(FATAL_ERROR "DirectX 12 is only supported on Windows platforms")
    endif ()
    target_compile_definitions(Engine PUBLIC RENDER_API_DX12)
    target_link_libraries(Engine PUBLIC d3d12.lib dxgi.lib dxguid.lib)
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
                      bshc_core
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
add_custom_command(
        TARGET Engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets"
        COMMENT "Copying assets to runtime output directory"
)
# ----------------------------------------------------------------------------