#include "pch.h"
#include "GameInput.h"
#include "GameState.h"
#include "WindowManager.h"
#include "Timeline.h"
#include "GameWorld.h"
#include "NamedSprite.h"

#include <iostream>

GameInput::GameInput()
    : pause_key(SDL_SCANCODE_P),
    scaling_key(SDL_SCANCODE_S),
    quit_key(SDL_SCANCODE_Q),
    speedUp_key(SDL_SCANCODE_EQUALS),
    speedDown_key(SDL_SCANCODE_MINUS),
    dash_key(SDL_SCANCODE_X),   // Set dash to 'X'
    left_key(SDL_SCANCODE_LEFT),
    right_key(SDL_SCANCODE_RIGHT),
    player_name("Player")  // default player name
{}

SDL_Scancode GameInput::getPauseKey() const { return pause_key; }
SDL_Scancode GameInput::getScalingKey() const { return scaling_key; }
SDL_Scancode GameInput::getQuitKey() const { return quit_key; }
SDL_Scancode GameInput::getSpeedUpKey() const { return speedUp_key; }
SDL_Scancode GameInput::getSpeedDownKey() const { return speedDown_key; }
const std::string& GameInput::getPlayerName() const { return player_name; }

void GameInput::setPlayerName(const std::string& name) { player_name = name; }
void GameInput::setPauseKey(SDL_Scancode key) { pause_key = key; }
void GameInput::setScalingKey(SDL_Scancode key) { scaling_key = key; }
void GameInput::setQuitKey(SDL_Scancode key) { quit_key = key; }
void GameInput::setSpeedUpKey(SDL_Scancode key) { speedUp_key = key; }
void GameInput::setSpeedDownKey(SDL_Scancode key) { speedDown_key = key; }

double GameInput::adjustTimeScale(double currentScale, double adjustment, double minScale, double maxScale) {
    double newScale = currentScale + adjustment;
    if (newScale > maxScale) {
        return maxScale;
    }
    if (newScale < minScale) {
        return minScale;
    }
    return newScale;
}

// private helper function to adjust global time scale
void GameInput::adjustGlobalTime(GameState& gameState, double adjustment) {
    Timeline& globalTimeline = gameState.getTimeline();
    GameWorld& gameWorld = gameState.getGameWorld();

    double newGlobalTimeScale = adjustTimeScale(globalTimeline.getTimeScale(), adjustment, 0.5, 2.0);
    globalTimeline.setTimeScale(newGlobalTimeScale);

    for (auto& pair : gameWorld.getObjects()) {
        NamedSprite* sprite = pair.second;
        Timeline& spriteTimeline = sprite->getTimeline();
        double newSpriteTimeScale = adjustTimeScale(spriteTimeline.getTimeScale(), adjustment, 0.5, 2.0);
        spriteTimeline.setTimeScale(newSpriteTimeScale);
    }
}

bool GameInput::handleInput(const Uint8* keystate, GameState& gameState) {
    bool quit = false;

    // Handle quit
    if (keystate[quit_key]) {
        quit = true;
    }

    // Toggle scaling
    if (keystate[scaling_key]) {
        gameState.getWindowManager().toggleProportionalMode();
    }

    // Pause or unpause the game
    if (keystate[pause_key]) {
        Timeline& globalTimeline = gameState.getTimeline();
        if (globalTimeline.isPaused()) {
            globalTimeline.unpause();
        }
        else {
            globalTimeline.pause();
        }
    }

    // Adjust time scale for the player sprite
    double adjustment = 0.0;
    if (keystate[speedUp_key]) {
        adjustment = 0.1;
    }
    else if (keystate[speedDown_key]) {
        adjustment = -0.1;
    }

    if (adjustment != 0.0) {
        NamedSprite* playerSprite = gameState.getGameWorld().getObjectByName(player_name);
        if (playerSprite) {
            Timeline& playerTimeline = playerSprite->getTimeline();
            double newPlayerTimeScale = adjustTimeScale(playerTimeline.getTimeScale(), adjustment, 0.5, 2.0);
            playerTimeline.setTimeScale(newPlayerTimeScale);
        }
    }

    return quit;
}

std::pair<bool, int> GameInput::handleInputFPS(const Uint8* keystate, GameState& gameState, int targetFps) {
    NamedSprite* playerSprite = gameState.getGameWorld().getObjectByName(player_name);
    Timeline& playerTimeline = playerSprite->getTimeline();
    bool quit = handleInput(keystate, gameState);
    int newFps = static_cast<int>(playerTimeline.getTimeScale() * targetFps);
    //std::cout << "timescale: " << playerTimeline.getTimeScale() << " newFps: " << newFps << std::endl;
    return { quit, newFps };
}


bool GameInput::isLeftDoubleTap() {
    const Uint8* state = SDL_GetKeyboardState(nullptr);
    auto now = std::chrono::steady_clock::now();

    if (state[left_key]) {
        if (!leftKeyHeld) {  // Only proceed if key was not already held
            leftKeyHeld = true;  // Mark the key as held

            // Measure time since last tap
            auto timeSinceLastTap = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastLeftTap).count();

            if (timeSinceLastTap < doubleTapThreshold) {
                leftTapCount++;
            }
            else {
                leftTapCount = 1;  // Reset count if time exceeds threshold
            }

            lastLeftTap = now;  // Update the last tap time

            if (leftTapCount == 2) {  // Confirm double-tap if two taps within threshold
                leftTapCount = 0;
                return true;
            }
        }
    }
    else {
        leftKeyHeld = false;  // Reset key held state when key is released
    }

    return false;
}

bool GameInput::isRightDoubleTap() {
    const Uint8* state = SDL_GetKeyboardState(nullptr);
    auto now = std::chrono::steady_clock::now();

    if (state[right_key]) {
        if (!rightKeyHeld) {  // Only proceed if key was not already held
            rightKeyHeld = true;

            auto timeSinceLastTap = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastRightTap).count();

            if (timeSinceLastTap < doubleTapThreshold) {
                rightTapCount++;
            }
            else {
                rightTapCount = 1;
            }

            lastRightTap = now;

            if (rightTapCount == 2) {
                rightTapCount = 0;
                return true;
            }
        }
    }
    else {
        rightKeyHeld = false;  // Reset when key is released
    }

    return false;
}

