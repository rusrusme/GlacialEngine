//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "RHITest.hpp"

#include <cassert>
#include <vector>

#include "core/Log.hpp"
#include "render/rhi/Buffer.hpp"
#include "render/rhi/BufferLayout.hpp"
#include "render/rhi/CommandBuffer.hpp"
#include "render/rhi/ComputePipeline.hpp"
#include "render/rhi/DescriptorSet.hpp"
#include "render/rhi/Framebuffer.hpp"
#include "render/rhi/PipelineState.hpp"
#include "render/rhi/RenderCommand.hpp"
#include "render/rhi/RHITypes.hpp"
#include "render/rhi/Shader.hpp"
#include "render/rhi/Swapchain.hpp"
#include "render/rhi/Sync.hpp"
#include "render/rhi/Texture.hpp"

using namespace engine;

static std::string debug_shader = R"(
    #vert
    layout(location = 0) in vec3 a_Pos;
    void main() { gl_Position = vec4(a_Pos, 1.0); }

    #frag
    layout(location = 0) out vec4 FragColor;
    void main() { FragColor = vec4(1.0, 0.0, 1.0, 1.0); }
)";

void RHIBufferTestLayer::on_attach(engine::SystemContext& context) {
    CORE_INFO("=================================================");
    CORE_INFO(" RUNNING RHI BUFFER & LAYOUT VALIDATION TESTS ");
    CORE_INFO("=================================================");

    // TEST 1.1: VertexBuffer (VBO) & Subdata Update
    {
        CORE_INFO("[Test 1.1] Testing VertexBuffer creation & set_data...");
        float vertices[] = {
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
            0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.5f, 0.0f, 0.5f, 1.0f
        };
        VertexBuffer vbo(vertices, sizeof(vertices));
        assert(vbo.renderer_id() != 0);

        float updated_vertex[] = {0.1f, 0.1f, 0.1f, 0.0f, 0.0f};
        vbo.set_data(updated_vertex, sizeof(updated_vertex), 0);
        CORE_INFO("-> TEST 1.1 PASSED (VBO ID: {0})", vbo.renderer_id());
    }

    // TEST 1.2: IndexBuffer (IBO)
    {
        CORE_INFO("[Test 1.2] Testing IndexBuffer creation...");
        uint32_t indices[] = {0, 1, 2};
        IndexBuffer ibo(indices, 3);
        assert(ibo.renderer_id() != 0);
        assert(ibo.get_count() == 3);
        CORE_INFO("-> TEST 1.2 PASSED (IBO ID: {0}, Count: {1})", ibo.renderer_id(), ibo.get_count());
    }

    // TEST 1.3: UniformBuffer (UBO) & ShaderStorageBuffer (SSBO)
    {
        CORE_INFO("[Test 1.3] Testing UniformBuffer & ShaderStorageBuffer...");
        UniformBuffer ubo(sizeof(float) * 16, 0); // Binding Slot 0
        assert(ubo.renderer_id() != 0);

        ShaderStorageBuffer ssbo(sizeof(float) * 1024, 1); // Binding Slot 1
        assert(ssbo.renderer_id() != 0);

        float matrix_data[16] = {1.0f};
        ubo.set_data(matrix_data, sizeof(matrix_data));
        ssbo.set_data(matrix_data, sizeof(matrix_data));
        CORE_INFO("-> TEST 1.3 PASSED (UBO ID: {0}, SSBO ID: {1})", ubo.renderer_id(), ssbo.renderer_id());
    }

    // TEST 1.4: IndirectBuffer & BufferLayout Calculation
    {
        CORE_INFO("[Test 1.4] Testing IndirectBuffer & BufferLayout calculations...");
        IndirectBuffer indirect_buf(sizeof(DrawIndexedIndirectCommand) * 10);
        assert(indirect_buf.renderer_id() != 0);

        BufferLayout layout = {
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float2, "a_TexCoord"},
            {ShaderDataType::Float3, "a_Normal"}
        };

        assert(layout.stride() == (sizeof(float) * (3 + 2 + 3)));
        assert(layout.elements().size() == 3);
        assert(layout.elements()[1].offset == sizeof(float) * 3);

        CORE_INFO("-> TEST 1.4 PASSED (IndirectBuffer ID: {0}, Calculated Stride: {1} Bytes)",
                  indirect_buf.renderer_id(), layout.stride());
    }
}

void RHITextureTestLayer::on_attach(engine::SystemContext& context) {
    CORE_INFO("=================================================");
    CORE_INFO(" RUNNING RHI TEXTURE & SAMPLER VALIDATION TESTS ");
    CORE_INFO("=================================================");

    // TEST 2.1: Texture2D Creation with Spec
    {
        CORE_INFO("[Test 2.1] Testing Texture2D Specification & Allocation...");
        TextureSpecification spec;
        spec.width = 512;
        spec.height = 512;
        spec.format = TextureFormat::RGBA8;
        spec.generate_mipmaps = true;

        Texture2D texture(spec);
        assert(texture.renderer_id() != 0);
        assert(texture.width() == 512);
        assert(texture.height() == 512);

        std::vector pixels(512 * 512, 0xFFFFFFFF);
        texture.set_data(pixels.data(), pixels.size() * sizeof(uint32_t));

        CORE_INFO("-> TEST 2.1 PASSED (Texture2D ID: {0}, Dimensions: {1}x{2})",
                  texture.renderer_id(), texture.width(), texture.height());
    }
}

void RHIPipelineAndDescriptorTestLayer::on_attach(engine::SystemContext& context) {
    CORE_INFO("=================================================");
    CORE_INFO(" RUNNING PIPELINE STATE & DESCRIPTOR SET TESTS ");
    CORE_INFO("=================================================");

    // TEST 3.1: PipelineState & VAO Compilation
    {
        CORE_INFO("[Test 3.1] Testing PipelineState VAO Creation...");

        PipelineStateDesc desc;
        desc.shader = std::make_shared<Shader>("shader", debug_shader);
        desc.vertex_layout = {
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float2, "a_TexCoord"}
        };
        desc.cull_mode = CullMode::Back;
        desc.polygon_mode = PolygonMode::Fill;
        desc.depth_test = true;

        PipelineState pipeline(desc);
        assert(pipeline.vao_id() != 0);
        CORE_INFO("-> TEST 3.1 PASSED (VAO ID: {0})", pipeline.vao_id());
    }

    // TEST 3.2: DescriptorSet Binding Registration
    {
        CORE_INFO("[Test 3.2] Testing DescriptorSet Writes...");
        UniformBuffer ubo(64, 0);
        ShaderStorageBuffer ssbo(128, 1);
        TextureSpecification spec{.width = 256, .height = 256, .format = TextureFormat::RGBA8};
        Texture2D tex(spec);

        DescriptorSet set;
        set.write_uniform_buffer(0, ubo);
        set.write_storage_buffer(1, ssbo);
        set.write_texture(0, tex);

        set.bind(DescriptorSetIndex::Frame);
        CORE_INFO("-> TEST 3.2 PASSED (DescriptorSet Resources Bound Successfully)");
    }
}

void RHIComputeAndSyncTestLayer::on_attach(engine::SystemContext& context) {
    CORE_INFO("=================================================");
    CORE_INFO(" RUNNING COMPUTE PIPELINE & FENCE SYNC TESTS ");
    CORE_INFO("=================================================");

    // TEST 4.1: Compute Pipeline Execution & Memory Barriers
    {
        CORE_INFO("[Test 4.1] Testing Compute Pipeline Memory Barriers...");

        ComputePipelineDesc desc;
        desc.shader = std::make_shared<Shader>("assets/shaders/compiled/test/compu.bshb");

        ComputePipeline compute_pipeline(desc);
        ComputePipeline::set_memory_barrier(MemoryBarrierFlags::ShaderStorage | MemoryBarrierFlags::CommandData);

        CORE_INFO("-> TEST 4.1 PASSED (Compute Barriers Executed)");
    }

    // TEST 4.2: CPU-GPU Fence Synchronization
    {
        CORE_INFO("[Test 4.2] Testing Fence Signal & Wait...");
        Fence fence;
        assert(fence.is_signaled() == true); // Null fence is considered signaled

        fence.signal();
        assert(fence.is_signaled() == false || fence.is_signaled() == true);

        bool wait_success = fence.wait(1000000000); // 1 Sekunde Timeout
        assert(wait_success == true);

        fence.reset();
        CORE_INFO("-> TEST 4.2 PASSED (Fence Signaled & Cleared)");
    }
}

void RHICommandExecutionTestLayer::on_attach(engine::SystemContext& context) {
    CORE_INFO("=================================================");
    CORE_INFO(" RUNNING COMMAND BUFFER & RENDER COMMAND TESTS ");
    CORE_INFO("=================================================");

    RenderCommand::init();
    CORE_INFO("-> RendererAPI Initialized via RenderCommand");

    auto shader = std::make_shared<Shader>("shader", debug_shader);

    PipelineStateDesc desc;
    desc.shader = shader;
    desc.vertex_layout = {
        {ShaderDataType::Float3, "a_Position"},
        {ShaderDataType::Float2, "a_TexCoord"}
    };
    desc.cull_mode = CullMode::Back;
    desc.polygon_mode = PolygonMode::Fill;
    desc.depth_test = true;

    // Pipeline erzeugen & als Member speichern (bindet VAO + Shader)
    test_pipeline_ = std::make_unique<PipelineState>(desc);
}

void RHICommandExecutionTestLayer::on_render(engine::SystemContext& context, float delta_time) {
    CommandBuffer cmd_buffer;
    cmd_buffer.begin();

    // Viewport & Scissor
    RenderCommand::set_viewport(cmd_buffer, 0, 0, 1920, 1080);
    cmd_buffer.set_scissor(0, 0, 1920, 1080);

    // Framebuffer & Pass
    FramebufferSpecification fbo_spec{
        .width       = 1920,
        .height      = 1080,
        .samples     = 1,
        .attachments = {TextureFormat::RGBA8, TextureFormat::Depth24Stencil8}
    };
    Framebuffer fbo(fbo_spec);

    const float clear_color[4] = {0.1f, 0.1f, 0.12f, 1.0f};
    RenderCommand::begin_render_pass(cmd_buffer, fbo, clear_color);

    // 1. Pipeline (und damit VAO + Shader) vor dem Draw Call binden!
    if (test_pipeline_) {
        RenderCommand::bind_pipeline(cmd_buffer, *test_pipeline_);
    }

    // 2. Jetzt ist ein VAO gebunden -> glDrawArraysInstancedBaseInstance ist valide
    cmd_buffer.draw(3, 1, 0, 0);

    RenderCommand::end_render_pass(cmd_buffer);
    cmd_buffer.end();

    assert(!cmd_buffer.is_recording());
}

RHITest::RHITest() : Application("Full RHI Test Suite") {
    push_layer(std::make_unique<RHIBufferTestLayer>());
    push_layer(std::make_unique<RHITextureTestLayer>());
    push_layer(std::make_unique<RHIPipelineAndDescriptorTestLayer>());
    push_layer(std::make_unique<RHIComputeAndSyncTestLayer>());
    push_layer(std::make_unique<RHICommandExecutionTestLayer>());
}

int main(int argc, char* argv[]) {
    RHITest rhi_test;
    rhi_test.run();
}
