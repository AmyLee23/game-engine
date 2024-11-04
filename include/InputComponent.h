#pragma once

#include "PositionComponent.h"
#include "GameState.h"
#include <SDL.h>

class NamedSprite;

class InputComponent {
public:
    virtual ~InputComponent() = default;

    // virtual void handleInput(const Uint8* keystate, PositionComponent* position, GameState& gameState, double dt) = 0;
    virtual void handleInput(const Uint8* keystate, NamedSprite& sprite, GameState& gameState, double dt) = 0;
};