//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "RmlRenderInterface.hpp"

#include "render/Renderer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace engine {
    RmlRenderInterface::RmlRenderInterface(Renderer& renderer) : renderer_(renderer) {}

    Rml::CompiledGeometryHandle RmlRenderInterface::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) {
        auto* geom = new RmlGeometry();
        geom->num_indices = static_cast<int>(indices.size());

        glGenVertexArrays(1, &geom->vao);
        glGenBuffers(1, &geom->vbo);
        glGenBuffers(1, &geom->ebo);

        glBindVertexArray(geom->vao);

        glBindBuffer(GL_ARRAY_BUFFER, geom->vbo);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(Rml::Vertex)), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geom->ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(int)), indices.data(), GL_STATIC_DRAW);

        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Rml::Vertex), reinterpret_cast<void*>(offsetof(Rml::Vertex, position)));

        // Color
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Rml::Vertex), reinterpret_cast<void*>(offsetof(Rml::Vertex, colour)));

        // UV
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Rml::Vertex), reinterpret_cast<void*>(offsetof(Rml::Vertex, tex_coord)));

        glBindVertexArray(0);

        return reinterpret_cast<Rml::CompiledGeometryHandle>(geom);
    }

    void RmlRenderInterface::RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture) {
        auto* geom = reinterpret_cast<RmlGeometry*>(geometry);
        if (!geom || geom->num_indices == 0) return;

        const auto ui_shader = renderer_.get_shader_library().get("ui");
        if (!ui_shader) return;

        ui_shader->bind();
        ui_shader->set_mat4("u_projection", projection_);
        ui_shader->set_vec2("u_translation", {translation.x, translation.y});

        if (texture != 0) {
            ui_shader->set_int("u_use_texture", 1);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(texture));
            ui_shader->set_int("u_texture", 0);
        }
        else {
            ui_shader->set_int("u_use_texture", 0);
        }

        glBindVertexArray(geom->vao);
        glDrawElements(GL_TRIANGLES, geom->num_indices, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
        ui_shader->unbind();
    }

    void RmlRenderInterface::ReleaseGeometry(Rml::CompiledGeometryHandle geometry) {
        auto* geom = reinterpret_cast<RmlGeometry*>(geometry);
        if (!geom) return;

        glDeleteVertexArrays(1, &geom->vao);
        glDeleteBuffers(1, &geom->vbo);
        glDeleteBuffers(1, &geom->ebo);
        delete geom;
    }

    void RmlRenderInterface::EnableScissorRegion(bool enable) {
        if (enable)
            glEnable(GL_SCISSOR_TEST);
        else
            glDisable(GL_SCISSOR_TEST);
    }

    void RmlRenderInterface::SetScissorRegion(Rml::Rectanglei region) {
        const int x = region.p0.x;
        const int y = region.p0.y;
        const int width = region.p1.x - region.p0.x;
        const int height = region.p1.y - region.p0.y;

        glScissor(x, viewport_height_ - (y + height), width, height);
    }

    Rml::TextureHandle RmlRenderInterface::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) {
        int w, h, channels;
        stbi_set_flip_vertically_on_load(false);
        unsigned char* data = stbi_load(source.c_str(), &w, &h, &channels, 4);
        if (!data) return 0;

        texture_dimensions = {w, h};
        Rml::TextureHandle handle = GenerateTexture(Rml::Span<const unsigned char>(data, w * h * 4), texture_dimensions);
        stbi_image_free(data);
        return handle;
    }

    Rml::TextureHandle RmlRenderInterface::GenerateTexture(Rml::Span<const unsigned char> source, Rml::Vector2i source_dimensions) {
        GLuint tex_id = 0;
        glGenTextures(1, &tex_id);
        glBindTexture(GL_TEXTURE_2D, tex_id);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, source_dimensions.x, source_dimensions.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, source.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        return tex_id;
    }

    void RmlRenderInterface::ReleaseTexture(Rml::TextureHandle texture_handle) {
        auto tex_id = static_cast<GLuint>(texture_handle);
        glDeleteTextures(1, &tex_id);
    }

    void RmlRenderInterface::set_viewport(const uint32_t width, const uint32_t height) {
        viewport_height_ = height;
        projection_ = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, -1.0f, 1.0f);
    }
}
