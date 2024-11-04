#pragma once

#include "MovementComponent.h"
#include "InputComponent.h"
#include "GameInput.h"
#include <mutex>
#include <chrono>

class ControllableJumper : public MovementComponent, public InputComponent {
private:
    double dx;            // Horizontal movement speed
    double dy;            // Vertical movement speed
    bool isJumping;      // Indicates if the sprite is currently jumping
    bool isOnGround;     // Indicates if the sprite is on the ground
    double jumpVelocity;  // The velocity when jumping
    std::mutex mtx;
    std::chrono::steady_clock::time_point lastDashTime;
    bool isDashing = false;  // Flag to indicate if currently dashing

public:
    ControllableJumper(double dx, double dy, double jumpVelocity);

    double getDx() const override;
    double getDy() const override;
    double getJumpVelocity() const;
    bool getIsJumping() const;
    bool getIsOnGround() const;

    void setDx(double dx) override;
    void setDy(double dy) override;
    void setJumpVelocity(double jumpVelocity);
    void setIsJumping(bool isJumping);
    void setIsOnGround(bool isOnGround);

    void move(NamedSprite& sprite, GameState& gameState, double dt) override;
    void handleInput(const Uint8* keystate, NamedSprite& sprite, GameState& gameState, double dt) override;
    void applyDashEffect(double dashMultiplier, GameInput& gameInput, bool dashingLeft);

};
