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
#include <vector>

#include "RenderPass.hpp"
#include "render/rhi/RHITypes.hpp"

namespace engine {
    class CommandBuffer;
    class Texture2D;

    class RenderGraphBuilder {
    public:
        void read_texture(const std::string& name);
        void write_texture(const std::string& name);
        void create_transient_texture(const std::string& name, uint32_t width, uint32_t height);
    };

    class RenderGraphRegistry {
    public:
        [[nodiscard]] Texture2D* texture(const std::string& name) const;
    };

    class RenderGraph {
    public:
        explicit RenderGraph(size_t initial_arena_bytes = 1024 * 1024)
            : arena_resource_(initial_arena_bytes),
              passes_(&arena_resource_) {}

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

        void compile();
        void execute(CommandBuffer& cmd, uint32_t frame_index);

        void reset() {
            for (RenderPass* pass : passes_) {
                std::destroy_at(pass);
            }
            passes_.clear();
            arena_resource_.release(); // O(1) wipe of arena allocations
        }

        [[nodiscard]] std::span<RenderPass* const> passes() const noexcept {
            return passes_;
        }

    private:
        std::pmr::monotonic_buffer_resource arena_resource_;
        std::pmr::vector<RenderPass*> passes_;
        RenderGraphRegistry registry_;
    };
}
