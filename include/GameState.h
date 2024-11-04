#pragma once

#include <SDL.h>

class Timeline;
class GameWorld;
class PhysicsEngine;
class WindowManager;
class GameTypes;

class GameState {
private:
    Timeline& globalTimeline;
    GameWorld& gameWorld;
    PhysicsEngine& physicsEngine;
    WindowManager& windowManager;
    GameTypes& gameTypes;

public:
    GameState(Timeline& timeline, GameWorld& world, PhysicsEngine& physics, WindowManager& window, GameTypes& types);
    ~GameState();

    void step(const Uint8* keystate);
    // step split up (for multi threading)
    void stepInput(const Uint8* keystate);
    void stepMovement();


    Timeline& getTimeline();
    GameWorld& getGameWorld();
    PhysicsEngine& getPhysicsEngine();
    WindowManager& getWindowManager();
    GameTypes& getGameTypes();
};