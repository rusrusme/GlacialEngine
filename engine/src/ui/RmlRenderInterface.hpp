//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <RmlUi/Core/RenderInterface.h>

namespace engine {
    class Renderer;

    struct RmlGeometry {
        GLuint vao = 0;
        GLuint vbo = 0;
        GLuint ebo = 0;
        int num_indices = 0;
    };

    class RmlRenderInterface : public Rml::RenderInterface {
    public:
        explicit RmlRenderInterface(Renderer& renderer);
        ~RmlRenderInterface() override = default;

        // --- Geometry ---
        Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) override;
        void RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture) override;
        void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override;

        // --- Scissoring ---
        void EnableScissorRegion(bool enable) override;
        void SetScissorRegion(Rml::Rectanglei region) override;

        // --- Textures ---
        Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) override;
        Rml::TextureHandle GenerateTexture(Rml::Span<const unsigned char> source, Rml::Vector2i source_dimensions) override;
        void ReleaseTexture(Rml::TextureHandle texture_handle) override;

        // Custom Helper
        void set_viewport(uint32_t width, uint32_t height);

    private:
        Renderer& renderer_;
        glm::mat4 projection_{1.0f};
        uint32_t viewport_height_ = 0;
    };
}
