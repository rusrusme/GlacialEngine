//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Sandbox.hpp"

#include "SandboxLayer.hpp"

Sandbox::Sandbox() : Application("Sandbox") {
    push_layer(std::make_unique<SandboxLayer>());
}

Sandbox::~Sandbox() = default;

void Sandbox::init(SystemContext& context) {
}

int main() {
    Sandbox sandbox;
    sandbox.run();
}
