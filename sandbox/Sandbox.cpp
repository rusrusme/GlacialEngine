//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Sandbox.hpp"

#include "Imgui.hpp"
#include "SandboxLayer.hpp"

Sandbox::Sandbox() : Application("Sandbox") {
    push_layer(std::make_unique<SandboxLayer>());
    push_overlay(std::make_unique<Imgui>());
}

Sandbox::~Sandbox() = default;

int main() {
    Sandbox sandbox;
    sandbox.run();
}
