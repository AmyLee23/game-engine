#include "pch.h"
#include "ControllableFlyer.h"
#include "PhysicsEngine.h"
#include "GameWorld.h"
#include "NamedSprite.h"

ControllableFlyer::ControllableFlyer(double dx, double dy)
    : dx(dx), dy(dy) {}

void ControllableFlyer::handleInput(const Uint8* keystate, NamedSprite& sprite, GameState& gameState, double dt) {
    PositionComponent* position = sprite.getPosition();
    // Store the previous position
    double lastX = position->getX();
    double lastY = position->getY();

    PhysicsEngine& physics = gameState.getPhysicsEngine();
    GameWorld& gameWorld = gameState.getGameWorld();

    // Calculate movement in meters based on velocity and time (dt)
    double metersMovedX = dx * dt;
    double metersMovedY = dy * dt;

    // Convert meters to pixels using the physics engine's pixels per meter
    double moveX = metersMovedX * physics.getPixelsPerMeter();
    double moveY = metersMovedY * physics.getPixelsPerMeter();

    // Handle Y movement
    if (keystate[SDL_SCANCODE_UP]) {
        position->setY(position->getY() - moveY);
    }
    if (keystate[SDL_SCANCODE_DOWN]) {
        position->setY(position->getY() + moveY);
    }
    if (position->getY() < 0 || position->getY() + position->getHeight() > gameWorld.getWorldHeight() ||
        physics.checkCollisions(sprite, gameState)) {
        // Revert Y position if collision occurs
        position->setY(lastY);
    }

    // Handle X movement
    if (keystate[SDL_SCANCODE_LEFT]) {
        position->setX(position->getX() - moveX);
    }
    if (keystate[SDL_SCANCODE_RIGHT]) {
        position->setX(position->getX() + moveX);
    }
    if (position->getX() < 0 || position->getX() + position->getWidth() > gameWorld.getWorldWidth() ||
        physics.checkCollisions(sprite, gameState)) {
        // Revert X position if collision occurs
        position->setX(lastX);
    }
}

double ControllableFlyer::getDx() const {
    return dx;
}

double ControllableFlyer::getDy() const {
    return dy;
}

void ControllableFlyer::setDx(double dx) {
    this->dx = dx;
}

void ControllableFlyer::setDy(double dy) {
    this->dy = dy;
}
