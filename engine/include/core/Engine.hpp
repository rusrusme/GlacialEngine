//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

// Asset
#include "asset/AssetManager.hpp"
#include "asset/AssetType.hpp"
#include "asset/MetafileHandler.hpp"
// Audio
#include "audio/AudioSystem.hpp"
// Core
#include "core/Application.hpp"
#include "core/Config.hpp"
#include "core/Layer.hpp"
#include "core/LayerStack.hpp"
#include "core/Log.hpp"
#include "asset/UUID.hpp"
#include "core/Window.hpp"
#include "core/WindowAPI.hpp"
// Event
#include "event/Event.hpp"
#include "event/KeyEvents.hpp"
#include "event/MouseEvents.hpp"
#include "event/WindowEvents.hpp"
// Input
#include "input/CameraController.hpp"
#include "input/Input.hpp"
#include "input/KeyCodes.hpp"
#include "input/MouseCodes.hpp"
// Math
#include "math/Ray.hpp"
#include "math/Raycaster.hpp"
// Physics
#include "physics/AABB.hpp"
#include "physics/CollisionSystem.hpp"
#include "physics/SpatialHashGrid.hpp"
// Platform
#include "platform/FileDialogAPI.hpp"
// Render/Debug
// #include "render/legacy/debug/DebugFrame.hpp"
// #include "render/legacy/debug/DebugRenderer.hpp"
// #include "render/legacy/debug/ImguiPlatform.hpp"
// // Render/Mesh
// #include "render/legacy/mesh/InstanceData.hpp"
// #include "render/legacy/mesh/Mesh.hpp"
// #include "render/legacy/mesh/MeshLoader.hpp"
// #include "render/legacy/mesh/PrimitiveMesh.hpp"
// // Render
// #include "../render/legacy/Buffer.hpp"
// #include "../render/legacy/BufferLayout.hpp"
// #include "render/legacy/Camera.hpp"
// #include "render/legacy/Frame.hpp"
// #include "render/legacy/Framebuffer.hpp"
// #include "render/legacy/Frustum.hpp"
// #include "render/legacy/PostProcessor.hpp"
// #include "../render/legacy/RenderAPI.hpp"
// #include "render/legacy/RenderCommand.hpp"
// #include "render/legacy/Renderer.hpp"
// #include "../render/legacy/RenderScope.hpp"
// #include "render/legacy/Shader.hpp"
// #include "../render/legacy/ShaderLibrary.hpp"
// #include "render/legacy/Texture2D.hpp"
// #include "../render/legacy/UniformBuffer.hpp"
// #include "../render/legacy/Vertex.hpp"
// #include "render/legacy/VertexArray.hpp"
// Scene/ECS
#include "project/scene/ecs/ComponentRegistry.hpp"
#include "project/scene/ecs/Components.hpp"
#include "project/scene/ecs/Entity.hpp"
// Scene/System
#include "project/scene/system/CameraSystem.hpp"
// Scene
#include "project/scene/Scene.hpp"
#include "project/scene/SceneManager.hpp"
#include "project/scene/SceneSerializer.hpp"
#include "project/scene/SceneState.hpp"
#include "project/scene/SimulationState.hpp"
// UI
#include "ui/UILayer.hpp"
#include "ui/UIManager.hpp"
// Util
#include "util/FileIO.hpp"
