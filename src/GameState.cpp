#include "pch.h"
#include "GameState.h"
#include "Timeline.h"
#include "GameWorld.h"
#include "PhysicsEngine.h"
#include "WindowManager.h"
#include "MovementComponent.h"
#include "InputComponent.h"
#include "GameTypes.h"
#include <iostream>

GameState::GameState(Timeline& timeline, GameWorld& world, PhysicsEngine& physics, WindowManager& window, GameTypes& types)
    : globalTimeline(timeline), gameWorld(world), physicsEngine(physics), windowManager(window), gameTypes(types) {}

GameState::~GameState() {
    for (auto& pair : gameWorld.getObjects()) {
        delete pair.second;
    }
}

void GameState::step(const Uint8* keystate) {
    stepInput(keystate);
    stepMovement();
}

void GameState::stepInput(const Uint8* keystate) {
    // Handle input for each NamedSprite
    for (const auto& pair : gameWorld.getObjects()) {
        NamedSprite* sprite = pair.second;

        // Handle input if the sprite has an InputComponent
        if (InputComponent* inputComponent = sprite->getInput()) {
            double dt = sprite->getTimeline().getInputDt();
            inputComponent->handleInput(keystate, *sprite, *this, dt);
        }
    }
}

void GameState::stepMovement() {
    // Handle movement for each NamedSprite
    for (const auto& pair : gameWorld.getObjects()) {
        NamedSprite* sprite = pair.second;

        // Handle movement if the sprite has a MovementComponent
        if (MovementComponent* movementComponent = sprite->getMovement()) {
            double dt = sprite->getTimeline().getMovementDt();
            movementComponent->move(*sprite, *this, dt);
        }
    }
}

Timeline& GameState::getTimeline() {
    return globalTimeline;
}

GameWorld& GameState::getGameWorld() {
    return gameWorld;
}

PhysicsEngine& GameState::getPhysicsEngine() {
    return physicsEngine;
}

WindowManager& GameState::getWindowManager() {
    return windowManager;
}

GameTypes& GameState::getGameTypes() {
    return gameTypes;
}
