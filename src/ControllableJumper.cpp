#include "pch.h"
#include "ControllableJumper.h"
#include "NamedSprite.h"
#include "PhysicsEngine.h"
#include "GameWorld.h"
#include "GameInput.h"
#include <iostream>
#include <chrono>
#include <thread>


ControllableJumper::ControllableJumper(double dx, double dy, double jumpVelocity)
    : dx(dx), dy(dy), isJumping(false), isOnGround(false), jumpVelocity(jumpVelocity) {}

double ControllableJumper::getDx() const { return dx; }
double ControllableJumper::getDy() const { return dy; }
double ControllableJumper::getJumpVelocity() const { return jumpVelocity; }
bool ControllableJumper::getIsJumping() const { return isJumping; }
bool ControllableJumper::getIsOnGround() const { return isOnGround; }

void ControllableJumper::setDx(double dx) { this->dx = dx; }
void ControllableJumper::setDy(double dy) { this->dy = dy; }
void ControllableJumper::setJumpVelocity(double jumpVelocity) { this->jumpVelocity = jumpVelocity; }
void ControllableJumper::setIsJumping(bool isJumping) { this->isJumping = isJumping; }
void ControllableJumper::setIsOnGround(bool isOnGround) { this->isOnGround = isOnGround; }

void ControllableJumper::move(NamedSprite& sprite, GameState& gameState, double dt) {
    std::lock_guard<std::mutex> lock(mtx);
    PositionComponent& position = *sprite.getPosition();
    PhysicsEngine& physics = gameState.getPhysicsEngine();

    double lastY = position.getY();

    // apply gravity (modify dy only)
    physics.applyGravityDy(sprite, dt);

    double moveY = dy * dt * physics.getPixelsPerMeter();
    double maxMove = physics.getMaxPixelMovement();

    // Calculate the number of steps needed to break down the movement
    int stepsY = static_cast<int>(std::ceil(std::abs(moveY) / maxMove));
    double incrementY = moveY / stepsY;

    for (int step = 0; step < stepsY; ++step) {
        position.setY(lastY - incrementY);

        // Check for collisions
        if (physics.checkCollisions(sprite, gameState)) {
            dy = 0;  // Stop vertical movement
            if (lastY < position.getY()) {
                isOnGround = true;  // If moving downwards and collided, is on ground
                position.setY(lastY);
                break;
            }
            position.setY(lastY);  // last valid position
        }
        else {
            lastY = position.getY();  // update lastY to the new valid position
        }
    }
}

void ControllableJumper::handleInput(const Uint8* keystate, NamedSprite& sprite, GameState& gameState, double dt) {
    std::lock_guard<std::mutex> lock(mtx);
    PositionComponent& position = *sprite.getPosition();
    GameWorld& gameWorld = gameState.getGameWorld();
    PhysicsEngine& physics = gameState.getPhysicsEngine();

    double lastX = position.getX();
    double lastY = position.getY();

    double metersMovedX = dx * dt;
    double metersMovedY = dy * dt;
    double moveX = metersMovedX * physics.getPixelsPerMeter();
    double moveY = metersMovedY * physics.getPixelsPerMeter();
    double maxMove = physics.getMaxPixelMovement();

    // calculate the number of steps needed
    int stepsX = static_cast<int>(std::ceil(std::abs(moveX) / maxMove));
    int stepsY = static_cast<int>(std::ceil(std::abs(moveY) / maxMove));
    int totalSteps = std::max(stepsX, stepsY);

    double incrementX = moveX / totalSteps;
    double incrementY = moveY / totalSteps;

    // handle jumping
    if (keystate[SDL_SCANCODE_SPACE] && isOnGround) {
        dy = jumpVelocity;  // start jumping by setting the vertical velocity
        isJumping = true;
        isOnGround = false;
    }

    // Incrementally handle movement
    for (int step = 0; step < totalSteps; ++step) {
        // increment horizontal movement
        if (keystate[SDL_SCANCODE_LEFT]) {
            position.setX(lastX - incrementX);
        }
        if (keystate[SDL_SCANCODE_RIGHT]) {
            position.setX(lastX + incrementX);
        }
        // check horizontal collisions
        if (position.getX() < 0 || position.getX() + position.getWidth() > gameWorld.getWorldWidth() || physics.checkCollisions(sprite, gameState)) {
            position.setX(lastX);  // reset to last valid position
            break;  // stop horizontal movement on collision
        }
        else {
            lastX = position.getX();  // Update lastX to the new valid position
        }
    }
}


void ControllableJumper::applyDashEffect(double dashMultiplier, GameInput& gameInput, bool dashingLeft) {
    std::lock_guard<std::mutex> lock(mtx);

    double originalDx = dx;
    dx *= dashMultiplier;

    if (dashingLeft) {
        gameInput.isDashingLeft = true;
    }
    else {
        gameInput.isDashingRight = true;
    }

    // Launch a thread to reset dx and the dash flags after the dash duration
    std::thread([this, &gameInput, originalDx, dashingLeft]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 0.5-second dash duration
        std::lock_guard<std::mutex> lock(this->mtx);
        this->dx = originalDx;

        // Reset dash flags after dash duration
        if (dashingLeft) {
            gameInput.isDashingLeft = false;
        }
        else {
            gameInput.isDashingRight = false;
        }
        }).detach();
}


/*
void ControllableJumper::move(NamedSprite& sprite, GameState& gameState, double dt) {
    PositionComponent& position = *sprite.getPosition();
    PhysicsEngine& physics = gameState.getPhysicsEngine();

    double lastY = position.getY();

    physics.applyGravity(sprite, dt);

    if (physics.checkCollisions(sprite, gameState)) {
        dy = 0;
        if (lastY < position.getY()) {
            isOnGround = true;
        }
        position.setY(lastY);
    }
    //std::cout << "postion y: " << position.getY() << ", dy: " << dy << std::endl;
}

void ControllableJumper::handleInput(const Uint8* keystate, NamedSprite& sprite, GameState& gameState, double dt) {
    PositionComponent& position = *sprite.getPosition();
    GameWorld& gameWorld = gameState.getGameWorld();

    double lastX = position.getX();
    double lastY = position.getY();
    PhysicsEngine& physics = gameState.getPhysicsEngine();

    double metersMovedX = dx * dt;  // Distance in meters (speed * time)
    double metersMovedY = dy * dt;
    double moveX = metersMovedX * physics.getPixelsPerMeter();
    double moveY = metersMovedY * physics.getPixelsPerMeter();

    // Handle jumping
    if (keystate[SDL_SCANCODE_SPACE] && isOnGround) {
        dy = jumpVelocity;  // start jumping by setting the vertical velocity
        isJumping = true;
        isOnGround = false;
    }
    if (keystate[SDL_SCANCODE_LEFT]) {
        position.setX(position.getX() - moveX);
    }
    if (keystate[SDL_SCANCODE_RIGHT]) {
        position.setX(position.getX() + moveX);
    }

    if (position.getX() < 0 || position.getX() + position.getWidth() > gameWorld.getWorldWidth() || physics.checkCollisions(sprite, gameState)) {
        position.setX(lastX);
    }
} */
