//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace engine {
    template<typename T>
    using Shared = std::shared_ptr<T>;

    template<typename T>
    using Unique = std::unique_ptr<T>;

    template<typename T>
    using Vector = std::vector<T>;

    enum class RendererBackend {
        None = 0,
        OpenGL,
        Vulkan,
        DX12
    };

    enum class ShaderDataType {
        None = 0,
        Float, Float2, Float3, Float4,
        Mat3, Mat4,
        Int, Int2, Int3, Int4,
        Bool
    };

    enum class TextureFormat {
        None = 0,
        RGBA8,
        RGB8,
        RGBA16F,
        RGBA32F,
        R32_UINT,
        Depth24Stencil8,
        Depth32F,
        R32F
    };

    enum class TextureFilter {
        Linear,
        Nearest,
        Anisotropic
    };

    enum class TextureWrap {
        Repeat,
        ClampToEdge,
        MirroredRepeat,
        Border
    };

    enum class CullMode {
        None = 0,
        Front,
        Back,
        FrontAndBack
    };

    enum class PolygonMode {
        Fill,
        Wireframe,
        Point
    };

    enum class BlendMode {
        Opaque = 0,
        Alpha,
        Additive
    };

    enum class CompareOp {
        Never = 0,
        Less,
        Equal,
        LessOrEqual,
        Greater,
        NotEqual,
        GreaterOrEqual,
        Always
    };

    enum class IndexType {
        UInt8,
        UInt16,
        UInt32
    };

    enum class DescriptorSetIndex : uint32_t {
        Frame    = 0, // Set 0: Per-Frame Uniforms (FrameData UBO, Time, View/Proj)
        Pass     = 1, // Set 1: Per-Pass Resources (Shadow Maps, G-Buffers, Light Lists)
        Material = 2  // Set 2: Per-Material Data (Albedo, Normal, Roughness)
    };

    struct DrawArraysIndirectCommand {
        uint32_t count{0};
        uint32_t instance_count{0};
        uint32_t first{0};
        uint32_t base_instance{0};
    };

    struct DrawIndexedIndirectCommand {
        uint32_t index_count{0};
        uint32_t instance_count{0};
        uint32_t first_index{0};
        int32_t vertex_offset{0};
        uint32_t first_instance{0};
    };

    enum class MemoryBarrierFlags : uint32_t {
        None                  = 0,
        ShaderStorage         = 1 << 0, // SSBO writes
        UniformBuffer         = 1 << 1, // UBO updates
        TextureFetch          = 1 << 2, // Storage Image / Texture writes
        CommandData           = 1 << 3, // Indirect Draw / Dispatch buffers
        FramebufferAttachment = 1 << 4, // Render Target barriers
        All                   = 0xFFFFFFFF
    };

    constexpr MemoryBarrierFlags operator|(MemoryBarrierFlags a, MemoryBarrierFlags b) {
        return static_cast<MemoryBarrierFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    constexpr MemoryBarrierFlags operator&(MemoryBarrierFlags a, MemoryBarrierFlags b) {
        return static_cast<MemoryBarrierFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
    }

    constexpr MemoryBarrierFlags operator^(MemoryBarrierFlags a, MemoryBarrierFlags b) {
        return static_cast<MemoryBarrierFlags>(static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b));
    }

    constexpr MemoryBarrierFlags operator~(MemoryBarrierFlags a) {
        return static_cast<MemoryBarrierFlags>(~static_cast<uint32_t>(a));
    }

    constexpr MemoryBarrierFlags& operator|=(MemoryBarrierFlags& a, MemoryBarrierFlags b) {
        a = a | b;
        return a;
    }

    constexpr MemoryBarrierFlags& operator&=(MemoryBarrierFlags& a, MemoryBarrierFlags b) {
        a = a & b;
        return a;
    }

    constexpr MemoryBarrierFlags& operator^=(MemoryBarrierFlags& a, MemoryBarrierFlags b) {
        a = a ^ b;
        return a;
    }

    struct SwapchainSpec {
        uint32_t width{1920};
        uint32_t height{1080};
        bool vsync{true};
        uint32_t max_frames_in_flight{2};
        void* window_handle{nullptr};
    };

    struct TextureSpecification {
        uint32_t width = 1;
        uint32_t height = 1;
        TextureFormat format = TextureFormat::RGBA8;
        TextureFilter min_filter = TextureFilter::Linear;
        TextureFilter mag_filter = TextureFilter::Linear;
        TextureWrap wrap_s = TextureWrap::Repeat;
        TextureWrap wrap_t = TextureWrap::Repeat;
        bool generate_mipmaps = true;
    };

    struct FramebufferAttachmentSpec {
        TextureFormat format = TextureFormat::None;

        FramebufferAttachmentSpec() = default;
        FramebufferAttachmentSpec(TextureFormat format) : format(format) {}
    };

    struct FramebufferSpecification {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t samples = 1;
        Vector<FramebufferAttachmentSpec> attachments;
        bool swapchain_target = false;
    };
}
