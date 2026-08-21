# engine/CMakeLists.txt
# ----------------------------------------------------------------------------
add_library(Engine STATIC
            src/core/Application.cpp
            include/core/Application.hpp
            src/render/Renderer.cpp
            include/render/Renderer.hpp
            src/render/DebugFrame.cpp
            include/render/DebugFrame.hpp
            src/math/Raycaster.cpp
            include/math/Raycaster.hpp
            src/render/Camera.cpp
            include/render/Camera.hpp
            include/math/Ray.hpp
            include/render/mesh/Mesh.hpp
            include/render/Vertex.hpp
            src/input/CameraController.cpp
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
            src/core/Layer.cpp
            include/core/Layer.hpp
            src/scene/ecs/Components.cpp
            include/scene/ecs/Components.hpp
            src/core/UUID.cpp
            include/core/Uuid.hpp
            include/core/Engine.hpp
            src/core/LayerStack.cpp
            include/core/LayerStack.hpp
            src/scene/system/CameraSystem.cpp
            include/scene/system/CameraSystem.hpp
            include/input/KeyCodes.hpp
            include/core/Window.hpp
            include/event/Event.hpp
            include/event/WindowEvents.hpp
            include/event/MouseEvents.hpp
            src/core/Window.cpp
            src/render/Buffer.cpp
            include/render/Buffer.hpp
            src/render/BufferLayout.cpp
            include/render/BufferLayout.hpp
            src/render/VertexArray.cpp
            include/render/VertexArray.hpp
            src/render/Shader.cpp
            include/render/Shader.hpp
            src/render/UniformBuffer.cpp
            include/render/UniformBuffer.hpp
            include/util/FileIo.hpp
            include/event/KeyEvents.hpp
            src/input/Input.cpp
            include/input/MouseCodes.hpp
            src/render/ShaderLibrary.cpp
            include/render/ShaderLibrary.hpp
            include/render/mesh/PrimitiveMesh.hpp
            src/scene/SceneSerializer.cpp
            include/scene/SceneSerializer.hpp
            src/render/mesh/MeshLoader.cpp
            include/render/mesh/MeshLoader.hpp
            src/render/mesh/Mesh.cpp
            src/render/mesh/Mesh.cpp
            src/asset/AssetType.cpp
            include/asset/AssetType.hpp
            src/asset/MetafileHandler.cpp
            include/asset/MetafileHandler.hpp
            include/asset/AssetManager.hpp
            src/asset/AssetManager.cpp
            include/physics/AABB.hpp
            include/physics/SpatialHashGrid.hpp
            include/physics/CollisionSystem.hpp
            src/physics/CollisionSystem.cpp
            src/physics/SpatialHashGrid.cpp
            include/render/mesh/InstanceData.hpp
)
# ----------------------------------------------------------------------------
target_include_directories(Engine
                           PUBLIC ${CMAKE_SOURCE_DIR}/engine/include
                           PRIVATE ${CMAKE_SOURCE_DIR}/engine/src
)
# ----------------------------------------------------------------------------
find_package(OpenGL REQUIRED)
target_link_libraries(Engine PUBLIC
                      glad
                      glfw
                      glm::glm
                      EnTT::EnTT
                      cgltf
                      OpenGL::GL
                      nlohmann_json::nlohmann_json
)
# ----------------------------------------------------------------------------
if (ENABLE_IMGUI)
    target_link_libraries(Engine PUBLIC imgui imguizmo::imguizmo)
endif ()

if (ENABLE_SPDLOG)
    target_link_libraries(Engine PUBLIC spdlog::spdlog)
endif ()
# ----------------------------------------------------------------------------
add_compile_definitions(GLFW_INCLUDE_NONE)
# ----------------------------------------------------------------------------
target_precompile_headers(Engine PRIVATE src/core/core_pch.hpp)
# ----------------------------------------------------------------------------

