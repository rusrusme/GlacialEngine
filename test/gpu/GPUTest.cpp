//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "GPUTest.hpp"

#include "render/gpu/GPUCuller.hpp"
#include "render/gpu/GPUData.hpp"
#include "render/gpu/HZBGenerator.hpp"
#include "render/rhi/Buffer.hpp"
#include "render/rhi/Texture.hpp"

void GPUTestLayer::on_attach(SystemContext& context) {
    CORE_INFO("=================================================");
    CORE_INFO(" RUNNING GPU PIPELINE & CULLING VALIDATION TESTS ");
    CORE_INFO("=================================================");

    // 1. Initialisiere Pipelines
    culler_ = std::make_unique<GPUCuller>();
    hzb_generator_ = std::make_unique<HZBGenerator>();

    // 2. Kamera-Daten aufbauen & UBO befüllen
    GPUCameraData camera_data{};
    camera_data.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    camera_data.projection = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 100.0f);
    camera_data.view_projection = camera_data.projection * camera_data.view;
    camera_data.camera_position = glm::vec4(0.0f, 0.0f, 5.0f, 1.0f);

    // Standard Frustum Planes für Sichtbarkeitstests (Ebenen: Ax + By + Cz + D = 0)
    camera_data.frustum_planes[0] = glm::vec4(1.0f, 0.0f, 0.0f, 10.0f);  // Left
    camera_data.frustum_planes[1] = glm::vec4(-1.0f, 0.0f, 0.0f, 10.0f); // Right
    camera_data.frustum_planes[2] = glm::vec4(0.0f, 1.0f, 0.0f, 10.0f);  // Bottom
    camera_data.frustum_planes[3] = glm::vec4(0.0f, -1.0f, 0.0f, 10.0f); // Top
    camera_data.frustum_planes[4] = glm::vec4(0.0f, 0.0f, -1.0f, 0.1f);  // Near
    camera_data.frustum_planes[5] = glm::vec4(0.0f, 0.0f, 1.0f, 100.0f); // Far

    camera_buffer_ = std::make_unique<UniformBuffer>(sizeof(GPUCameraData), 0);
    camera_buffer_->set_data(&camera_data, sizeof(GPUCameraData));

    // 3. Test-Instanzen aufbauen (Eine im Frustum, eine weit außerhalb)
    std::vector<GPUInstanceData> instances(2);

    // Instanz 0: Im Sichtfeld (Center = [0,0,0], Radius = 1.0)
    instances[0].model_matrix = glm::mat4(1.0f);
    instances[0].bounding_center_radius = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    instances[0].mesh_id = 0;

    // Instanz 1: Weit außerhalb (Center = [500,0,0], Radius = 1.0) -> Soll ge-culled werden!
    instances[1].model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(500.0f, 0.0f, 0.0f));
    instances[1].bounding_center_radius = glm::vec4(500.0f, 0.0f, 0.0f, 1.0f);
    instances[1].mesh_id = 1;

    instance_buffer_ = std::make_unique<ShaderStorageBuffer>(sizeof(GPUInstanceData) * instances.size(), 0);
    instance_buffer_->set_data(instances.data(), sizeof(GPUInstanceData) * instances.size());

    // 4. Indirect Draw Commands vorbereiten
    std::vector<GPUDrawCommand> commands(2);
    commands[0] = {.count = 36, .instance_count = 0, .first_index = 0, .base_vertex = 0, .base_instance = 0};
    commands[1] = {.count = 36, .instance_count = 0, .first_index = 0, .base_vertex = 0, .base_instance = 1};

    indirect_buffer_ = std::make_unique<IndirectBuffer>(sizeof(GPUDrawCommand) * commands.size());
    indirect_buffer_->set_data(commands.data(), sizeof(GPUDrawCommand) * commands.size());

    // 5. Texturen für HZB-Test vorbereiten
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
        .format           = TextureFormat::RGBA32F, // Oder R32F
        .generate_mipmaps = true
    };
    hzb_target_tex_ = std::make_unique<Texture2D>(hzb_spec);

    CORE_INFO("-> GPU Test Resources Allocation Complete");
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
