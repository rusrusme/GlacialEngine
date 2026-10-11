//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "core/Application.hpp"
#include "render/rhi/Buffer.hpp"
#include "render/rhi/Texture.hpp"

namespace engine {
    class GPUCuller;
    class HZBGenerator;
}

using namespace engine;

class GPUTestLayer : public Layer {
public:
    void on_attach(SystemContext& context) override;
    void on_render(SystemContext& context, float delta_time) override;

private:
    std::unique_ptr<GPUCuller> culler_;
    std::unique_ptr<HZBGenerator> hzb_generator_;

    std::unique_ptr<UniformBuffer> camera_buffer_;
    std::unique_ptr<ShaderStorageBuffer> instance_buffer_;
    std::unique_ptr<IndirectBuffer> indirect_buffer_;

    std::unique_ptr<Texture2D> source_depth_tex_;
    std::unique_ptr<Texture2D> hzb_target_tex_;
};

class GPUTest : public Application {
public:
    GPUTest();
    ~GPUTest() override = default;
};
