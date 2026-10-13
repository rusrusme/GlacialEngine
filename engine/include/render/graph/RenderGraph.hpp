//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <concepts>
#include <memory>
#include <memory_resource>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

#include "RenderPass.hpp"
#include "render/rhi/CommandBuffer.hpp"
#include "render/rhi/RHITypes.hpp"
#include "render/rhi/Texture.hpp"

namespace engine {
    class RenderGraphRegistry {
    public:
        [[nodiscard]] Texture2D* texture(const std::string& name) const;
        void register_texture(const std::string& name, Texture2D* texture_ptr);
        void clear();

    private:
        std::unordered_map<std::string, Texture2D*> textures_;
    };

    class RenderGraph {
    public:
        explicit RenderGraph(size_t initial_arena_bytes = 1024 * 1024);
        ~RenderGraph() { reset(); }

        RenderGraph(const RenderGraph&) = delete;
        RenderGraph& operator=(const RenderGraph&) = delete;
        RenderGraph(RenderGraph&&) noexcept = delete;
        RenderGraph& operator=(RenderGraph&&) noexcept = delete;

        /**
         * @brief Allocates and constructs a RenderPass in the frame arena.
         * Enforces derived type and valid constructor arguments at compile-time.
         */
        template<typename T, typename... Args>
            requires std::derived_from<T, RenderPass> && std::constructible_from<T, Args...>
        T* add_pass(Args&&... args) {
            void* memory = arena_resource_.allocate(sizeof(T), alignof(T));
            T* pass = std::construct_at(static_cast<T*>(memory), std::forward<Args>(args)...);

            passes_.push_back(pass);
            return pass;
        }

        void import_texture(const std::string& name, Texture2D* texture_ptr);

        void compile();
        void execute(CommandBuffer& command, uint32_t frame_index);

        void reset();

        [[nodiscard]] std::span<RenderPass* const> passes() const noexcept { return passes_; }

        [[nodiscard]] RenderGraphRegistry& registry() noexcept { return registry_; }

    private:
        friend class RenderGraphBuilder;

        struct TransientTextureDesc {
            uint32_t width{0};
            uint32_t height{0};
            TextureFormat format{TextureFormat::RGBA8};
        };

        std::pmr::monotonic_buffer_resource arena_resource_;
        std::pmr::vector<RenderPass*> passes_;
        RenderGraphRegistry registry_;

        std::unordered_map<std::string, Unique<Texture2D>> transient_textures_;
        std::unordered_map<std::string, TransientTextureDesc> transient_specs_;
    };

    class RenderGraphBuilder {
    public:
        explicit RenderGraphBuilder(RenderGraph& graph, RenderPass& pass)
            : graph_(graph), pass_(pass) {}

        void read_texture(const std::string& name);
        void write_texture(const std::string& name);
        void create_transient_texture(const std::string& name, uint32_t width, uint32_t height, TextureFormat format = TextureFormat::RGBA8);

    private:
        RenderGraph& graph_;
        RenderPass& pass_;
    };
}
