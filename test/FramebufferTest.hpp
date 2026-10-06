//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "core/Application.hpp"
#include "core/Layer.hpp"

class FramebufferTestLayer : public engine::Layer {
public:
    FramebufferTestLayer();

    void on_attach(engine::SystemContext& context) override;
};

class TestApplication : public engine::Application {
public:
    TestApplication();
};
