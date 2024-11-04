#pragma once

#include "InputComponent.h"
#include "PositionComponent.h"
#include "GameState.h"
#include <SDL.h>

class ControllableFlyer : public InputComponent {
private:
    double dx;  // Movement speed in the x direction
    double dy;  // Movement speed in the y direction

public:
    ControllableFlyer(double dx, double dy);

    void handleInput(const Uint8* keystate, NamedSprite& sprite, GameState& gameState, double dt) override;

    double getDx() const;
    double getDy() const;

    void setDx(double dx);
    void setDy(double dy);
};
