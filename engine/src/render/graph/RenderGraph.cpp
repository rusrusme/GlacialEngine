//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/graph/RenderGraph.hpp"

#include <concepts>
#include <memory>
#include <memory_resource>
#include <string>
#include <unordered_map>
#include <vector>

namespace engine {
    Texture2D* RenderGraphRegistry::texture(const std::string& name) const {
        const auto it = textures_.find(name);
        if (it != textures_.end()) {
            return it->second;
        }
        return nullptr;
    }

    void RenderGraphRegistry::register_texture(const std::string& name, Texture2D* texture_ptr) {
        textures_[name] = texture_ptr;
    }

    void RenderGraphRegistry::clear() {
        textures_.clear();
    }

    RenderGraph::RenderGraph(size_t initial_arena_bytes) : arena_resource_(initial_arena_bytes),
                                                           passes_(&arena_resource_) {}

    void RenderGraph::import_texture(const std::string& name, Texture2D* texture_ptr) {
        registry_.register_texture(name, texture_ptr);
    }

    void RenderGraph::compile() {
        for (RenderPass* pass : passes_) {
            if (!pass || !pass->is_enabled()) {
                continue;
            }

            RenderGraphBuilder builder(*this, *pass);
            pass->setup(builder);
        }

        for (const auto& [name, spec] : transient_specs_) {
            TextureSpecification tex_spec{};
            tex_spec.width = spec.width;
            tex_spec.height = spec.height;
            tex_spec.format = spec.format;
            tex_spec.min_filter = TextureFilter::Linear;
            tex_spec.mag_filter = TextureFilter::Linear;
            tex_spec.generate_mipmaps = false;

            auto tex = std::make_unique<Texture2D>(tex_spec);
            registry_.register_texture(name, tex.get());
            transient_textures_[name] = std::move(tex);
        }
    }

    void RenderGraph::execute(CommandBuffer& command, uint32_t frame_index) {
        const RenderContext context{.command_buffer = command, .current_frame_index = frame_index};

        for (RenderPass* pass : passes_) {
            if (!pass || !pass->is_enabled()) {
                continue;
            }

            // Automatische Memory Barriers zwischen den Pass-Ausführungen setzen
            command.memory_barrier(MemoryBarrierFlags::FramebufferAttachment | MemoryBarrierFlags::ShaderStorage);

            pass->execute(context, registry_);
        }
    }

    void RenderGraph::reset() {
        for (RenderPass* pass : passes_) {
            std::destroy_at(pass);
        }
        passes_.clear();
        transient_textures_.clear();
        registry_.clear();
        arena_resource_.release(); // O(1) wipe aller Arena-Allokationen
    }

    void RenderGraphBuilder::read_texture(const std::string& name) {
    }

    void RenderGraphBuilder::write_texture(const std::string& name) {
    }

    void RenderGraphBuilder::create_transient_texture(const std::string& name, uint32_t width, uint32_t height, TextureFormat format) {
        graph_.transient_specs_[name] = RenderGraph::TransientTextureDesc{.width = width, .height = height, .format = format};
    }
}
