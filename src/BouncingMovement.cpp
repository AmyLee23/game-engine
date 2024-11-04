#include "pch.h"
#include "BouncingMovement.h"
#include "PhysicsEngine.h"
#include "GameWorld.h"
#include "NamedSprite.h"
#include <iostream>

BouncingMovement::BouncingMovement(double dx, double dy)
    : dx(dx), dy(dy) {}

void BouncingMovement::move(NamedSprite& sprite, GameState& gameState, double dt) {
    std::lock_guard<std::mutex> lock(mtx);

    PositionComponent* position = sprite.getPosition();
    PhysicsEngine& physics = gameState.getPhysicsEngine();
    GameWorld& gameWorld = gameState.getGameWorld();

    double metersMovedX = dx * dt;
    double metersMovedY = dy * dt;
    double moveX = metersMovedX * physics.getPixelsPerMeter();
    double moveY = metersMovedY * physics.getPixelsPerMeter();

    double maxMove = physics.getMaxPixelMovement(); // max movement increment

    // calculate the number of steps needed to break down the movement
    int stepsX = static_cast<int>(std::ceil(std::abs(moveX) / maxMove));
    int stepsY = static_cast<int>(std::ceil(std::abs(moveY) / maxMove));

    // iterate over greater number
    int totalSteps = std::max(stepsX, stepsY);

    // incremental movement for each step
    double incrementX = moveX / totalSteps;
    double incrementY = moveY / totalSteps;

    double lastX = position->getX();
    double lastY = position->getY();

    for (int step = 0; step < totalSteps; ++step) {
        // try incremental x movement
        position->setX(lastX + incrementX);

        if (position->getX() < 0 || position->getX() + position->getWidth() > gameWorld.getWorldWidth()) {
            // reverse horizontal velocity (bounce)
            dx = -dx;
            position->setX(lastX);  // reset to last valid position
        }
        else if (physics.checkCollisions(sprite, gameState)) {
            dx = -dx;
            position->setX(lastX);  // reset to last valid position
        }
        else {
            lastX = position->getX();  // update lastX to the new valid position
        }

        // try incremental y movement
        position->setY(lastY + incrementY);

        if (position->getY() < 0 || position->getY() + position->getHeight() > gameWorld.getWorldHeight()) {
            // reverse vertical velocity (bounce)
            dy = -dy;
            position->setY(lastY);  // reset to last valid position
        }
        else if (physics.checkCollisions(sprite, gameState)) {
            dy = -dy;
            position->setY(lastY);  // reset to last valid position
        }
        else {
            lastY = position->getY();  // update lastY to the new valid position
        }
    }
}


/*
void BouncingMovement::move(NamedSprite& sprite, GameState& gameState, double dt) {
    PositionComponent* position = sprite.getPosition();
    PhysicsEngine& physics = gameState.getPhysicsEngine();
    GameWorld& gameWorld = gameState.getGameWorld();

    double metersMovedX = dx * dt;
    double metersMovedY = dy * dt;
    double moveX = metersMovedX * physics.getPixelsPerMeter();
    double moveY = metersMovedY * physics.getPixelsPerMeter();
    //std::cout << "Move x: " << moveX << std::endl;
    double lastX = position->getX();
    double lastY = position->getY();

    // try x movement
    position->setX(lastX + moveX);
    // Check for collisions with GameWorld bounds
    if (position->getX() < 0 || position->getX() + position->getWidth() > gameWorld.getWorldWidth()) {
        // If collision occurs on the left or right side, reverse horizontal velocity (bounce)
        //std::cout << "x collision world: " << (lastX + moveX) << std::endl;
        dx = -dx;
        position->setX(lastX);
    } else if (physics.checkCollisions(sprite, gameState)) {
        //std::cout << "x collision shape: " << (lastX + moveX) << std::endl;
        dx = -dx;  // Bounce horizontally
        position->setX(lastX);
    }

    //try y movement
    position->setY(lastY + moveY);
    if (position->getY() < 0 || position->getY() + position->getHeight() > gameWorld.getWorldHeight()) {
        // If collision occurs on the top or bottom side, reverse vertical velocity (bounce)
        dy = -dy;
        position->setY(lastY);
    } else if (physics.checkCollisions(sprite, gameState)) {
        dy = -dy;  // Bounce vertically
        position->setY(lastY);
    }
} */

double BouncingMovement::getDx() const { return dx; }
double BouncingMovement::getDy() const { return dy; }

void BouncingMovement::setDx(double dx) { this->dx = dx; }
void BouncingMovement::setDy(double dy) { this->dy = dy; }
