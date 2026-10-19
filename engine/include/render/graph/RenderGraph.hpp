//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>
#include <memory_resource>
#include <string>
#include <unordered_map>
#include <vector>

#include "render/rhi/CommandBuffer.hpp"
#include "render/rhi/RHITypes.hpp"
#include "render/rhi/Texture.hpp"

// TODO Temporary setup
namespace engine {
    class RenderGraph;
    class RenderGraphRegistry;
    class RenderGraphBuilder;

    struct RenderPassNode {
        std::string_view name;
        void* payload{nullptr};
        void (*execute_fn)(void* payload, const RenderGraphRegistry& registry, CommandBuffer& cb){nullptr};
    };

    class RenderGraphRegistry {
    public:
        [[nodiscard]] Texture2D* texture(const std::string& name) const;
        void register_texture(const std::string& name, Texture2D* texture_ptr);
        void clear();

    private:
        std::unordered_map<std::string, Texture2D*> textures_;
    };

    class RenderGraphBuilder {
    public:
        explicit RenderGraphBuilder(RenderGraph& graph, std::string_view pass_name)
            : graph_(graph), pass_name_(pass_name) {}

        void read_texture(const std::string& name);
        void write_texture(const std::string& name);
        void create_transient_texture(const std::string& name, uint32_t width, uint32_t height, TextureFormat format = TextureFormat::RGBA8);

    private:
        RenderGraph& graph_;
        std::string_view pass_name_;
    };

    class RenderGraph {
    public:
        explicit RenderGraph(size_t initial_arena_bytes = 1024 * 1024)
            : arena_resource_(initial_arena_bytes), passes_(&arena_resource_) {}

        ~RenderGraph() { reset(); }

        RenderGraph(const RenderGraph&) = delete;
        RenderGraph& operator=(const RenderGraph&) = delete;
        RenderGraph(RenderGraph&&) noexcept = delete;
        RenderGraph& operator=(RenderGraph&&) noexcept = delete;

        /**
         * @brief Registriert einen Pass via Lambdas ohne dynamische Vererbung.
         */
        template<typename PassData, typename SetupFn, typename ExecuteFn>
            requires std::is_invocable_v<SetupFn, RenderGraphBuilder&, PassData&> &&
            std::is_invocable_v<ExecuteFn, const PassData&, const RenderGraphRegistry&, CommandBuffer&>
        void add_pass(std::string_view name, SetupFn&& setup, ExecuteFn&& execute) {
            struct PassPayload {
                PassData data;
                std::decay_t<ExecuteFn> execute_lambda;
            };

            void* mem = arena_resource_.allocate(sizeof(PassPayload), alignof(PassPayload));
            auto* payload = std::construct_at(
                static_cast<PassPayload*>(mem),
                PassData{},
                std::forward<ExecuteFn>(execute)
            );

            RenderGraphBuilder builder(*this, name);
            setup(builder, payload->data);

            auto trampoline = [](void* ptr, const RenderGraphRegistry& registry, CommandBuffer& cb) {
                auto* p = static_cast<PassPayload*>(ptr);
                p->execute_lambda(p->data, registry, cb);
            };

            passes_.push_back(RenderPassNode{
                .name       = name,
                .payload    = payload,
                .execute_fn = trampoline
            });
        }

        void compile();

        void execute(CommandBuffer& command, uint32_t frame_index) {
            for (const auto& pass : passes_) {
                pass.execute_fn(pass.payload, registry_, command);
            }
        }

        void reset() {
            passes_.clear();
            transient_textures_.clear();
            registry_.clear();
            arena_resource_.release();
        }

    private:
        friend class RenderGraphBuilder;

        struct TransientTextureDesc {
            uint32_t width{0};
            uint32_t height{0};
            TextureFormat format{TextureFormat::RGBA8};
        };

        std::pmr::monotonic_buffer_resource arena_resource_;
        std::pmr::vector<RenderPassNode> passes_;
        RenderGraphRegistry registry_;

        std::unordered_map<std::string, Unique<Texture2D>> transient_textures_;
        std::unordered_map<std::string, TransientTextureDesc> transient_specs_;
    };
}
