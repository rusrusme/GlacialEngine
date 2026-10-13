//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "core/Application.hpp"

class ShaderTestLayer : public engine::Layer {
public:
    ShaderTestLayer() : Layer("Shader Test Layer") {}
    void on_attach(engine::SystemContext& context) override;
};

class ShaderSystemTest : public engine::Application {
public:
    ShaderSystemTest() : Application("Shader System Test Suite"){
        push_layer(std::make_unique<ShaderTestLayer>());
    }
};
