#include "WindowManager.h"
#include "PhysicsEngine.h"
#include "GameWorld.h"
#include "Timeline.h"
#include "GameState.h"
#include "NamedSprite.h"
#include "StandardPosition.h"
#include "BouncingMovement.h"
#include "ControllableFlyer.h"
#include "SimpleRectDrawer.h"
#include "ControllableJumper.h"
#include "MovementComponent.h"
#include "GameInput.h"
#include "GameTypes.h"
#include "EventManager.h"

#include <thread>
#include <fstream>
#include <json.hpp>

int main(int argc, char* args[]) {
    // Load sprite data from JSON
    std::ifstream inputFile("initial_game_state.json");
    nlohmann::json jsonData;
    inputFile >> jsonData;

    // game initialization
    WindowManager windowManager;
    PhysicsEngine physics;
    Timeline globalTimeline(nullptr, 1);
    GameWorld gameWorld(800, 600);
    GameTypes gameTypes; 
    GameState gameState(globalTimeline, gameWorld, physics, windowManager, gameTypes);
    GameInput gameInput;
    EventManager eventManager(gameState, gameInput);

    // other configs
    gameState.getPhysicsEngine().setPixelsPerMeter(16.0);
    gameState.getPhysicsEngine().setGravity(5.0 / 100.0);
    

    // create window
    if (!windowManager.init(900, 700)) {
        std::cerr << "Failed to initialize!" << std::endl;
        return -1;
    }

    // Iterate through the sprite data and create the sprites
    for (const auto& spriteData : jsonData["sprites"]) {
        std::string name = spriteData["name"];
        SDL_Color color = {
            spriteData["color"]["r"],
            spriteData["color"]["g"],
            spriteData["color"]["b"],
            spriteData["color"]["a"]
        };
        PositionComponent* position = new StandardPosition(
            spriteData["position"]["x"],
            spriteData["position"]["y"],
            spriteData["position"]["width"],
            spriteData["position"]["height"]
        );
        DrawerComponent* drawer = new SimpleRectDrawer(color);

        // Initialize the sprite's movement component
        MovementComponent* movement = nullptr;
        InputComponent* input = nullptr;

        if (name == "jumper") {
            // Controllable jumper movement
            ControllableJumper* jumperMovement = new ControllableJumper(45.0 / 100.0, 0.0, 1.5);
            movement = jumperMovement;
            input = jumperMovement;  // Assign input component to handle user input
            gameInput.setPlayerName(name);
        }
        else if (name.find("platform") != std::string::npos) {
        // Create a platform movement component with parameters from JSON
        float bounceSpeedX = spriteData["movementParams"]["bounceSpeed"];
        float bounceSpeedY = spriteData["movementParams"]["bounceHeight"];
        movement = new BouncingMovement(bounceSpeedX / 100.0, bounceSpeedY / 100.0); // Set both speeds based on JSON
    }

        // Create the NamedSprite with the appropriate movement component
        Timeline* localTimeline = new Timeline(&globalTimeline, spriteData["timeline"]);
        NamedSprite* sprite = new NamedSprite(name, 0, position, movement, input, drawer, *localTimeline);
        gameWorld.addObject(sprite);
    }

    gameInput.setPlayerName("jumper"); // Ensure this is set

    bool quit = false;
    SDL_Event e;
    const Uint8* keystate = SDL_GetKeyboardState(NULL);

    int targetFPS = 120;
    int64_t timePerFrame = 1000 / targetFPS;

    while (!quit) {
        int64_t frameStartTime = globalTimeline.getTime();

        while (SDL_PollEvent(&e) != 0) {
            // SDL quit
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            // handle game input
            else if (e.type == SDL_KEYDOWN) {
                quit = gameInput.handleInput(keystate, gameState);
            }
            // window resize
            if (e.type == SDL_WINDOWEVENT) {
                if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    int windowWidth = e.window.data1;
                    int windowHeight = e.window.data2;
                    windowManager.updateWindowSize(windowWidth, windowHeight, gameState);
                }
            }
        }
        gameState.step(keystate);
        // Define the respawn point
        const float respawnX = 100.0f; // X coordinate of respawn point
        const float respawnY = 400.0f; // Y coordinate of respawn point

        // **New Code**: Check for dash input chord and trigger dash event if active
        if (gameInput.isLeftDoubleTap()) {
            auto dashEvent = std::make_shared<Event>(globalTimeline, "DashLeft", 1);
            eventManager.raiseEvent(dashEvent);
        }
        else if (gameInput.isRightDoubleTap()) {
            auto dashEvent = std::make_shared<Event>(globalTimeline, "DashRight", 1);
            eventManager.raiseEvent(dashEvent);
        }

        // Process events, including the dash event
        eventManager.handleEvents();

        gameState.step(keystate);

        // Check if the jumper is off-screen
        NamedSprite* jumper = gameWorld.getObjectByName("jumper");
        if (jumper) {
            PositionComponent* jumperPosition = dynamic_cast<PositionComponent*>(jumper->getPosition());
            if (jumperPosition) {
                float jumperX = jumperPosition->getX();
                float jumperY = jumperPosition->getY();

                // Check if off-screen
                if (jumperX < 0 || jumperX > windowManager.getWindowWidth() ||
                    jumperY < 0 || jumperY > windowManager.getWindowHeight()) {
                    // Respawn at defined point
                    jumperPosition->setX(respawnX);
                    jumperPosition->setY(respawnY);
                }
            }
        }

        windowManager.updateWindow(gameState);

        int64_t frameEndTime = globalTimeline.getTime();
        int64_t frameDuration = frameEndTime - frameStartTime;
        int64_t sleepTime = timePerFrame - frameDuration;

        if (sleepTime > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
        }
    }

    windowManager.close();
    return 0;
}
