#pragma once

#include "PositionComponent.h"
#include "GameState.h"

class NamedSprite;

class MovementComponent {
public:
    virtual ~MovementComponent() = default;

    virtual double getDx() const = 0;
    virtual double getDy() const = 0;
    virtual void setDx(double dx) = 0;
    virtual void setDy(double dy) = 0;

    // virtual void move(PositionComponent* position, GameState& gameState, double dt) = 0;
    virtual void move(NamedSprite& sprite, GameState& gameState, double dt) = 0;
};