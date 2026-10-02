//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Game.hpp"

#include "GameLayer.hpp"

Game::Game() : Application("Game") {
    auto game_layer = std::make_unique<GameLayer>();
    push_layer(std::move(game_layer));
}

Game::~Game() = default;

int main() {
    Game game;
    game.run();
}

