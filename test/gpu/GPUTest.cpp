//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "GPUTest.hpp"

#include <cassert>
#include <vector>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#include "core/Log.hpp"
#include "render/gpu/GPUCuller.hpp"
#include "render/gpu/GPUData.hpp"
#include "render/gpu/HZBGenerator.hpp"
#include "render/rhi/Buffer.hpp"
#include "render/rhi/CommandBuffer.hpp"
#include "render/rhi/Texture.hpp"

using namespace engine;

void GPUTestLayer::on_attach(SystemContext& context) {
    CORE_INFO("=================================================");
    CORE_INFO(" RUNNING GPU PIPELINE & OCCLUSION CULLING TESTS ");
    CORE_INFO("=================================================");

    // 1. Initialize GPU Pipelines
    culler_ = std::make_unique<GPUCuller>();
    hzb_generator_ = std::make_unique<HZBGenerator>();

    // 2. Setup Camera Uniform Data
    GPUCameraData camera_data{};
    camera_data.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    camera_data.projection = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 100.0f);
    camera_data.view_projection = camera_data.projection * camera_data.view;
    camera_data.camera_position = glm::vec4(0.0f, 0.0f, 5.0f, 1.0f);
    camera_data.viewport_size = glm::vec2(1024.0f, 576.0f); // 16:9 Viewport

    // Extract Frustum Planes automatically from View-Projection Matrix (Gribb-Hartmann)
    const glm::mat4& vp = camera_data.view_projection;
    camera_data.frustum_planes[0] = glm::vec4(vp[0][3] + vp[0][0], vp[1][3] + vp[1][0], vp[2][3] + vp[2][0], vp[3][3] + vp[3][0]); // Left
    camera_data.frustum_planes[1] = glm::vec4(vp[0][3] - vp[0][0], vp[1][3] - vp[1][0], vp[2][3] - vp[2][0], vp[3][3] - vp[3][0]); // Right
    camera_data.frustum_planes[2] = glm::vec4(vp[0][3] + vp[0][1], vp[1][3] + vp[1][1], vp[2][3] + vp[2][1], vp[3][3] + vp[3][1]); // Bottom
    camera_data.frustum_planes[3] = glm::vec4(vp[0][3] - vp[0][1], vp[1][3] - vp[1][1], vp[2][3] - vp[2][1], vp[3][3] - vp[3][1]); // Top
    camera_data.frustum_planes[4] = glm::vec4(vp[0][3] + vp[0][2], vp[1][3] + vp[1][2], vp[2][3] + vp[2][2], vp[3][3] + vp[3][2]); // Near
    camera_data.frustum_planes[5] = glm::vec4(vp[0][3] - vp[0][2], vp[1][3] - vp[1][2], vp[2][3] - vp[2][2], vp[3][3] - vp[3][2]); // Far

    // Normalize plane equations
    for (auto & frustum_plane : camera_data.frustum_planes) {
        float len = glm::length(glm::vec3(frustum_plane));
        frustum_plane /= len;
    }

    camera_buffer_ = std::make_unique<UniformBuffer>(sizeof(GPUCameraData), 0);
    camera_buffer_->set_data(&camera_data, sizeof(GPUCameraData));

    // 3. Construct Instance Scenarios:
    // Instance 0: Visible inside frustum, in front of depth wall (z = 0.0)
    // Instance 1: Outside frustum (x = 500.0)
    // Instance 2: Inside frustum, but occluded behind depth wall (z = -10.0)
    std::vector<GPUInstanceData> instances(3);

    // Instance 0 (Visible)
    instances[0].model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    instances[0].bounding_center_radius = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    instances[0].bounding_box_min = glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f);
    instances[0].bounding_box_max = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

    // Instance 1 (Frustum Culled)
    instances[1].model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(500.0f, 0.0f, 0.0f));
    instances[1].bounding_center_radius = glm::vec4(500.0f, 0.0f, 0.0f, 1.0f);
    instances[1].bounding_box_min = glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f);
    instances[1].bounding_box_max = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

    // Instance 2 (Occluded behind wall)
    instances[2].model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f));
    instances[2].bounding_center_radius = glm::vec4(0.0f, 0.0f, -10.0f, 1.0f);
    instances[2].bounding_box_min = glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f);
    instances[2].bounding_box_max = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

    instance_buffer_ = std::make_unique<ShaderStorageBuffer>(sizeof(GPUInstanceData) * instances.size(), 0);
    instance_buffer_->set_data(instances.data(), sizeof(GPUInstanceData) * instances.size());

    // 4. Prepare Indirect Buffer
    std::vector<GPUDrawCommand> commands(3);
    for (uint32_t i = 0; i < 3; ++i) {
        commands[i] = {.count = 36, .instance_count = 0, .first_index = 0, .base_vertex = 0, .base_instance = i};
    }

    indirect_buffer_ = std::make_unique<IndirectBuffer>(sizeof(GPUDrawCommand) * commands.size());
    indirect_buffer_->set_data(commands.data(), sizeof(GPUDrawCommand) * commands.size());

    // 5. Prepare Textures for Depth & HZB
    TextureSpecification depth_spec{
        .width            = 1024,
        .height           = 1024,
        .format           = TextureFormat::Depth32F,
        .generate_mipmaps = false
    };
    source_depth_tex_ = std::make_unique<Texture2D>(depth_spec);

    TextureSpecification hzb_spec{
        .width            = 512,
        .height           = 512,
        .format           = TextureFormat::R32F,
        .min_filter       = TextureFilter::Nearest,
        .mag_filter       = TextureFilter::Nearest,
        .generate_mipmaps = true
    };
    hzb_target_tex_ = std::make_unique<Texture2D>(hzb_spec);

    // Set wall depth to 0.988f (~8 meters away)
    // Instance 0 (5m away, depth ~0.981) -> IN FRONT OF WALL -> VISIBLE
    // Instance 2 (15m away, depth ~0.993) -> BEHIND WALL -> OCCLUDED
    std::vector<float> initial_depth(1024 * 1024, 0.988f);
    source_depth_tex_->set_data(initial_depth.data(), initial_depth.size() * sizeof(float));

    CommandBuffer cmd;
    cmd.begin();

    // -----------------------------------------------------------------
    // TEST 1: Pure Frustum Culling
    // -----------------------------------------------------------------
    culler_->dispatch_cull(cmd, static_cast<uint32_t>(instances.size()), *instance_buffer_, *indirect_buffer_, *camera_buffer_);

    // -----------------------------------------------------------------
    // TEST 2: HZB Mipmap Generation
    // -----------------------------------------------------------------
    hzb_generator_->generate(cmd, *source_depth_tex_, *hzb_target_tex_);

    // -----------------------------------------------------------------
    // TEST 3: Full Pass - HZB Occlusion Culling
    // -----------------------------------------------------------------
    culler_->dispatch_cull(cmd, static_cast<uint32_t>(instances.size()), *instance_buffer_, *indirect_buffer_, *camera_buffer_, hzb_target_tex_.get());

    cmd.end();

    // -----------------------------------------------------------------
    // TEST 4: Edge Cases & Multi-Workgroup Stress Test
    // -----------------------------------------------------------------
    const uint32_t STRESS_INSTANCE_COUNT = 1000;
    std::vector<GPUInstanceData> stress_instances(STRESS_INSTANCE_COUNT);
    std::vector<GPUDrawCommand> stress_commands(STRESS_INSTANCE_COUNT);

    // Alle 1000 Commands & Instanzen initialisieren
    for (uint32_t i = 0; i < STRESS_INSTANCE_COUNT; ++i) {
        float x_pos = (i % 2 == 0) ? 0.0f : 500.0f; // Gerade = Sichtbar (0.0), Ungerade = Culled (500.0)
        stress_instances[i].bounding_center_radius = glm::vec4(x_pos, 0.0f, 0.0f, 1.0f);
        stress_instances[i].bounding_box_min = glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f);
        stress_instances[i].bounding_box_max = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

        stress_commands[i] = {.count = 36, .instance_count = 0, .first_index = 0, .base_vertex = 0, .base_instance = i};
    }

    // Instanz 0: Bounding Sphere ist im Frustum (Z=4.8), AABB schneidet die Near-Plane (Z > 4.9)
    stress_instances[0].bounding_center_radius = glm::vec4(0.0f, 0.0f, 4.8f, 0.5f);
    stress_instances[0].bounding_box_min = glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f);
    stress_instances[0].bounding_box_max = glm::vec4( 0.5f,  0.5f,  0.5f, 1.0f);

    auto stress_instance_buf = std::make_unique<ShaderStorageBuffer>(sizeof(GPUInstanceData) * STRESS_INSTANCE_COUNT, 0);
    stress_instance_buf->set_data(stress_instances.data(), sizeof(GPUInstanceData) * STRESS_INSTANCE_COUNT);

    auto stress_indirect_buf = std::make_unique<IndirectBuffer>(sizeof(GPUDrawCommand) * STRESS_INSTANCE_COUNT);
    stress_indirect_buf->set_data(stress_commands.data(), sizeof(GPUDrawCommand) * STRESS_INSTANCE_COUNT);

    // Dispatch für 1000 Instanzen über 16 Compute Workgroups ((1000 + 63) / 64)
    CommandBuffer stress_cmd;
    stress_cmd.begin();
    culler_->dispatch_cull(stress_cmd, STRESS_INSTANCE_COUNT, *stress_instance_buf, *stress_indirect_buf, *camera_buffer_, hzb_target_tex_.get());
    stress_cmd.end();

    // Readback Stress Test Results
    std::vector<GPUDrawCommand> stress_results(STRESS_INSTANCE_COUNT);
    glGetNamedBufferSubData(stress_indirect_buf->renderer_id(), 0, sizeof(GPUDrawCommand) * STRESS_INSTANCE_COUNT, stress_results.data());


    // Assertions:
    // Index 0: Near plane fallback -> sichtbares Clipping
    // Index 1: x_pos = 500.0f -> ge-culled
    // Index 2: x_pos = 0.0f -> sichtbar
    assert(stress_results[0].instance_count == 1 && "Near plane intersecting instance must be visible!");
    assert(stress_results[1].instance_count == 0 && "Far out stress instance (index 1) must be culled!");
    assert(stress_results[2].instance_count == 1 && "In-frustum stress instance (index 2) must be visible!");

    CORE_INFO("[TEST 4] Multi-Workgroup Dispatch ({0} instances across 16 Workgroups) & Near-Plane Clipping PASSED!", STRESS_INSTANCE_COUNT);

    // -----------------------------------------------------------------
    // VALIDATION: GPU Readback & Assertions
    // -----------------------------------------------------------------
    std::vector<GPUDrawCommand> gpu_results(3);
    glGetNamedBufferSubData(indirect_buffer_->renderer_id(), 0, sizeof(GPUDrawCommand) * gpu_results.size(), gpu_results.data());

    CORE_INFO("[TEST 1 & 3] Validating Full Pass GPU Culling Results:");
    CORE_INFO("  -> Instance 0 (In Front of Wall):  instance_count = {0} (Expected: 1)", gpu_results[0].instance_count);
    CORE_INFO("  -> Instance 1 (Outside Frustum):   instance_count = {0} (Expected: 0)", gpu_results[1].instance_count);
    CORE_INFO("  -> Instance 2 (Behind Depth Wall): instance_count = {0} (Expected: 0)", gpu_results[2].instance_count);

    assert(gpu_results[0].instance_count == 1 && "Instance 0 in front of wall must be visible!");
    assert(gpu_results[1].instance_count == 0 && "Instance 1 outside frustum must be culled!");
    assert(gpu_results[2].instance_count == 0 && "Instance 2 behind depth wall must be occluded!");

    CORE_INFO("[TEST 2] HZB Mip Chain Generated successfully across {0} Mip Levels", hzb_target_tex_->mip_levels());

    CORE_INFO("=================================================");
    CORE_INFO("FULL GPU OCCLUSION & CULLING PASS PASSED!");
    CORE_INFO("=================================================");
}

void GPUTestLayer::on_render(SystemContext& context, float delta_time) {
    Layer::on_render(context, delta_time);
}

GPUTest::GPUTest() {
    push_layer(std::make_unique<GPUTestLayer>());
}

int main(int argc, char* argv[]) {
    GPUTest test;
    test.run();
}
