//
// Copyright (c) 2026 rusrusme
// All rights reserved.
//

#include "FramebufferTest.hpp"

#include "core/Log.hpp"
#include "render/rhi/Framebuffer.hpp"

#include <cassert>

FramebufferTestLayer::FramebufferTestLayer() : Layer("FramebufferTestLayer") {}

void FramebufferTestLayer::on_attach(engine::SystemContext& context) {
    CORE_INFO("========================================");
    CORE_INFO(" RUNNING GLFRAMEBUFFER VALIDATION TESTS ");
    CORE_INFO("========================================");

    // -------------------------------------------------------------
    // TEST 1: Standard Main Render Target (1x Color RGBA8 + Depth24Stencil8)
    // -------------------------------------------------------------
    {
        CORE_INFO("[Test 1] Testing Standard Main Target (1x Color + Depth)...");
        engine::FramebufferSpecification spec;
        spec.width = 1920;
        spec.height = 1080;
        spec.attachments = {engine::TextureFormat::RGBA8, engine::TextureFormat::Depth24Stencil8};

        engine::Framebuffer fbo(spec);
        assert(fbo.renderer_id() != 0);
        assert(fbo.color_attachment_renderer_id(0) != 0);
        assert(fbo.depth_attachment_renderer_id() != 0);
        CORE_INFO("-> TEST 1 PASSED (FBO ID: {0})", fbo.renderer_id());
    }

    // -------------------------------------------------------------
    // TEST 2: Deferred G-Buffer / MRT (3x Color HDR + Depth32F)
    // -------------------------------------------------------------
    {
        CORE_INFO("[Test 2] Testing G-Buffer Multi-Render-Target (3x HDR Color + Depth)...");
        engine::FramebufferSpecification spec;
        spec.width = 1920;
        spec.height = 1080;
        spec.attachments = {
            engine::TextureFormat::RGBA16F,  // Albedo / Normal
            engine::TextureFormat::RGBA16F,  // PBR Material
            engine::TextureFormat::R32_UINT, // Entity ID Attachment
            engine::TextureFormat::Depth32F  // Depth
        };

        engine::GLFramebuffer fbo(spec);
        assert(fbo.renderer_id() != 0);
        assert(fbo.color_attachment_renderer_id(0) != 0);
        assert(fbo.color_attachment_renderer_id(1) != 0);
        assert(fbo.color_attachment_renderer_id(2) != 0);
        assert(fbo.depth_attachment_renderer_id() != 0);
        CORE_INFO("-> TEST 2 PASSED (FBO ID: {0})", fbo.renderer_id());
    }

    // -------------------------------------------------------------
    // TEST 3: Depth-Only Target (Shadow Map / CSM / Depth Prepass)
    // -------------------------------------------------------------
    {
        CORE_INFO("[Test 3] Testing Depth-Only Target (Shadow Map)...");
        engine::FramebufferSpecification spec;
        spec.width = 2048;
        spec.height = 2048;
        spec.attachments = {engine::TextureFormat::Depth32F};

        engine::GLFramebuffer fbo(spec);
        assert(fbo.renderer_id() != 0);
        assert(fbo.depth_attachment_renderer_id() != 0);
        CORE_INFO("-> TEST 3 PASSED (FBO ID: {0})", fbo.renderer_id());
    }

    // -------------------------------------------------------------
    // TEST 4: Resizing & VRAM Re-allocation
    // -------------------------------------------------------------
    {
        CORE_INFO("[Test 4] Testing Framebuffer Resize...");
        engine::FramebufferSpecification spec;
        spec.width = 1280;
        spec.height = 720;
        spec.attachments = {engine::TextureFormat::RGBA8, engine::TextureFormat::Depth24Stencil8};

        engine::GLFramebuffer fbo(spec);
        fbo.resize(1920, 1080);

        assert(fbo.specification().width == 1920);
        assert(fbo.specification().height == 1080);
        assert(fbo.renderer_id() != 0);
        assert(fbo.color_attachment_renderer_id(0) != 0);

        CORE_INFO("-> TEST 4 PASSED (Resized 720p -> 1080p, New FBO ID: {0})", fbo.renderer_id());
    }

    // -------------------------------------------------------------
    // TEST 5: Zero / Invalid Size Guard
    // -------------------------------------------------------------
    {
        CORE_INFO("[Test 5] Testing Zero Size Guard...");
        engine::FramebufferSpecification spec;
        spec.width = 1280;
        spec.height = 720;
        spec.attachments = {engine::TextureFormat::RGBA8};

        engine::GLFramebuffer fbo(spec);

        fbo.resize(0, 0);
        assert(fbo.specification().width == 1280); // Bleibt sicher auf altem Wert

        CORE_INFO("-> TEST 5 PASSED (Zero Size Safely Ignored)");
    }

    CORE_INFO("========================================");
    CORE_INFO(" ALL FRAMEBUFFER TESTS PASSED! ");
    CORE_INFO("========================================");
}

TestApplication::TestApplication() : Application("Framebuffer Test Suite") {
    push_layer(std::make_unique<FramebufferTestLayer>());
}

int main() {
    TestApplication application;
    return application.run();
}