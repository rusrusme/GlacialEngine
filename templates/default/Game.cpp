//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Game.hpp"

#include <GlacialEngine.hpp>

#include "GameLayer.hpp"

Game::Game() : Application("Game") {
    push_layer(std::make_unique<GameLayer>());
}

void Game::init(engine::SystemContext& context) {
}

int main() {
    Game game;
    game.run();
}

