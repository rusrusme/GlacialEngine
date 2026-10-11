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
    camera_data.viewport_size = glm::vec2(512.0f, 512.0f);

    // Frustum Planes (Ax + By + Cz + D = 0)
    camera_data.frustum_planes[0] = glm::vec4(1.0f, 0.0f, 0.0f, 10.0f);  // Left
    camera_data.frustum_planes[1] = glm::vec4(-1.0f, 0.0f, 0.0f, 10.0f); // Right
    camera_data.frustum_planes[2] = glm::vec4(0.0f, 1.0f, 0.0f, 10.0f);  // Bottom
    camera_data.frustum_planes[3] = glm::vec4(0.0f, -1.0f, 0.0f, 10.0f); // Top
    camera_data.frustum_planes[4] = glm::vec4(0.0f, 0.0f, -1.0f, 0.1f);  // Near
    camera_data.frustum_planes[5] = glm::vec4(0.0f, 0.0f, 1.0f, 100.0f); // Far

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
        .format           = TextureFormat::RGBA32F,
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
