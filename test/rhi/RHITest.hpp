//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "core/Application.hpp"
#include "render/rhi/PipelineState.hpp"

class RHIBufferTestLayer : public engine::Layer {
public:
    RHIBufferTestLayer() : Layer("RHI Buffer Test Layer") {}
    void on_attach(engine::SystemContext& context) override;
};

class RHITextureTestLayer : public engine::Layer {
public:
    RHITextureTestLayer() : Layer("RHI Texture Test Layer") {}
    void on_attach(engine::SystemContext& context) override;
};

class RHIPipelineAndDescriptorTestLayer : public engine::Layer {
public:
    RHIPipelineAndDescriptorTestLayer() : Layer("RHI Pipeline Descriptor Test Layer") {}
    void on_attach(engine::SystemContext& context) override;
};

class RHIComputeAndSyncTestLayer : public engine::Layer {
public:
    RHIComputeAndSyncTestLayer() : Layer("RHI Compute Sync Test Layer") {}
    void on_attach(engine::SystemContext& context) override;
};

class RHICommandExecutionTestLayer : public engine::Layer {
public:
    RHICommandExecutionTestLayer() : Layer("RHI Command Execution Test Layer") {}
    void on_attach(engine::SystemContext& context) override;
    void on_render(engine::SystemContext& context, float delta_time) override;
private:
    std::unique_ptr<engine::PipelineState> test_pipeline_;
};

class RHITest : public engine::Application {
public:
    RHITest();

    ~RHITest() override = default;
};
