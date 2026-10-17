//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Sandbox.hpp"

#include "Imgui.hpp"
#include "SandboxLayer.hpp"

Sandbox::Sandbox() : Application("Sandbox") {
    auto sandbox_layer = std::make_unique<SandboxLayer>();
    push_layer(std::move(sandbox_layer));
    push_overlay(std::make_unique<Imgui>());
}

Sandbox::~Sandbox() = default;

int main() {
    Sandbox sandbox;
    sandbox.run();
}
