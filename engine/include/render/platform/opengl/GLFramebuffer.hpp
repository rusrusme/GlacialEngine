//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/rhi/Framebuffer.hpp"
#include "render/rhi/RHITypes.hpp"

#include <cstdint>
#include <vector>

namespace engine {
    class GLFramebuffer {
    public:
        GLFramebuffer() = default;
        explicit GLFramebuffer(const FramebufferSpecification& spec);
        ~GLFramebuffer();

        GLFramebuffer(const GLFramebuffer&) = delete;
        GLFramebuffer& operator=(const GLFramebuffer&) = delete;

        GLFramebuffer(GLFramebuffer&& other) noexcept;
        GLFramebuffer& operator=(GLFramebuffer&& other) noexcept;

        void bind() const;
        void unbind() const;

        void resize(uint32_t width, uint32_t height);

        [[nodiscard]] uint32_t renderer_id() const { return renderer_id_; }
        [[nodiscard]] uint32_t color_attachment_renderer_id(uint32_t index = 0) const;
        [[nodiscard]] uint32_t depth_attachment_renderer_id() const { return depth_attachment_; }
        [[nodiscard]] const FramebufferSpecification& specification() const { return specification_; }

    private:
        void invalidate();
        void release();

        uint32_t renderer_id_ = 0;
        FramebufferSpecification specification_{};

        std::vector<FramebufferAttachmentSpec> color_attachment_specs_;
        FramebufferAttachmentSpec depth_attachment_spec_ = TextureFormat::None;

        std::vector<uint32_t> color_attachments_;
        uint32_t depth_attachment_ = 0;
    };
} // engine
