//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include "render/shader/BSHLoader.hpp"

namespace engine {
    class GLShader {
    public:
        GLShader() = default;
        /** @brief Uses .bshb or .bsh (Hot-reload) */
        explicit GLShader(std::string filepath);
        GLShader(std::string name, const std::string& shader_code);

        ~GLShader();

        GLShader(const GLShader&) = delete;
        GLShader& operator=(const GLShader&) = delete;

        GLShader(GLShader&& other) noexcept;
        GLShader& operator=(GLShader&& other) noexcept;

        bool reload();

        void bind() const;
        /** @brief Used for manual resets. Not intended to be inside render loop */
        void unbind() const;

        [[nodiscard]] const std::string& name() const { return name_; }
        [[nodiscard]] uint32_t renderer_id() const { return renderer_id_; }
        [[nodiscard]] const std::string& filepath() const { return filepath_; }

    private:
        void load(const LoadedBSHData& bsh_data);
        void create_program(const std::unordered_map<uint32_t, std::string>& sources);

        uint32_t renderer_id_ = 0;
        std::string name_;
        std::string filepath_;
    };
} // engine