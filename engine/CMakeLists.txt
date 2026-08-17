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
            include/render/Mesh.hpp
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
            src/scene/Entity.cpp
            include/scene/Entity.hpp
            src/core/Layer.cpp
            include/core/Layer.hpp
            src/scene/Components.cpp
            include/scene/Components.hpp
            src/core/UUID.cpp
            include/core/Uuid.hpp
            include/core/Engine.hpp
            src/core/LayerStack.cpp
            include/core/LayerStack.hpp
            src/scene/CameraSystem.cpp
            include/scene/CameraSystem.hpp
            include/core/Keys.hpp
            include/core/Window.hpp
            include/event/Event.hpp
            include/event/WindowResizedEvent.hpp
            include/event/MouseScrolledEvent.hpp
            src/core/Window.cpp
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
)
# ----------------------------------------------------------------------------
if (ENABLE_IMGUI)
    target_link_libraries(Engine PUBLIC imgui)
endif ()

if (NOT DIST_BUILD)
    target_link_libraries(Engine PUBLIC spdlog::spdlog)
endif ()
# ----------------------------------------------------------------------------
add_compile_definitions(GLFW_INCLUDE_NONE)
# ----------------------------------------------------------------------------

