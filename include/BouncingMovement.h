#pragma once

#include "MovementComponent.h"
#include "GameState.h"
#include <cmath>
#include <mutex>

class BouncingMovement : public MovementComponent {
private:
    std::mutex mtx;
    double dx;  // Movement in x direction (velocity)
    double dy;  // Movement in y direction (velocity)

public:
    BouncingMovement(double dx, double dy);

    void move(NamedSprite& sprite, GameState& gameState, double dt) override;

    double getDx() const override;
    double getDy() const override;

    void setDx(double dx) override;
    void setDy(double dy) override;
};