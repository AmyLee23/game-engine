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
#include "GameInput.h"

#include <thread>
#include <mutex>
#include <condition_variable> // unused at moment

// synchronization primitives
std::mutex mtx;

// thread function to handle input
void handleInput(GameState& gameState, const Uint8* keystate) {
    std::unique_lock<std::mutex> lock(mtx);
    gameState.stepInput(keystate);  // call stepInput
}

// thread function to handle movement
void handleMovement(GameState& gameState) {
    std::unique_lock<std::mutex> lock(mtx);
    gameState.stepMovement();  // call stepMovement
}

int main(int argc, char* args[]) {
    // game initialization
    WindowManager windowManager;
    PhysicsEngine physics;
    Timeline globalTimeline(nullptr, 1);
    GameWorld gameWorld(800, 600);
    GameState gameState(globalTimeline, gameWorld, physics, windowManager);

    // other configs
    gameState.getPhysicsEngine().setPixelsPerMeter(16.0);
    GameInput gameInput;

    // create window
    if (!windowManager.init(900, 700)) {
        std::cerr << "Failed to initialize!" << std::endl;
        return -1;
    }

    // colors
    SDL_Color green = { 0, 255, 0, 255 };
    SDL_Color red = { 255, 0, 0, 255 };

    // fixed shape
    PositionComponent* fp = new StandardPosition(0.0, static_cast<double>(gameWorld.getWorldHeight() - 50), gameWorld.getWorldWidth(), 50);
    DrawerComponent* fd = new SimpleRectDrawer(green);
    Timeline localTimeline1(&globalTimeline, 64);
    NamedSprite* ground = new NamedSprite("ground", fp, nullptr, nullptr, fd, localTimeline1);
    gameWorld.addObject(ground);

    // bouncing shape
    PositionComponent* bp = new StandardPosition(500.0, 200.0, 80, 60);
    MovementComponent* bm = new BouncingMovement(30.0 / 100.0, 0);
    DrawerComponent* bd = new SimpleRectDrawer(green);
    Timeline localTimeline2(&globalTimeline, 10);
    NamedSprite* platform = new NamedSprite("platform", bp, bm, nullptr, bd, localTimeline2);
    gameWorld.addObject(platform);

    // controllable jumper
    gameState.getPhysicsEngine().setGravity(5.0 / 100.0);
    PositionComponent* jp = new StandardPosition(100.0, 100.0, 50, 50);
    ControllableJumper* jmi = new ControllableJumper(45.0 / 100.0, 0.0, 1.5);  // handles both movement and input
    DrawerComponent* jd = new SimpleRectDrawer(red);
    Timeline localTimeline4(&globalTimeline, 10);
    NamedSprite* jumper = new NamedSprite("jumper", jp, jmi, jmi, jd, localTimeline4);
    gameWorld.addObject(jumper);
    gameInput.setPlayerName("jumper"); // THIS IS A MUST OR SPEEDING/SLOWING WILL NOT WORK

    bool quit = false;
    SDL_Event e;
    const Uint8* keystate = SDL_GetKeyboardState(NULL);

    int targetFPS = 120; // beware making this any lower than 60, as half speed/framerate will be buggy
    int64_t timePerFrame = 1000 / targetFPS;

    while (!quit) {
        int64_t frameStartTime = globalTimeline.getTime();
        int localFPS = targetFPS;

        while (SDL_PollEvent(&e) != 0) {
            // SDL quit
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            // handle game input
            else if (e.type == SDL_KEYDOWN) {
                auto pair = gameInput.handleInputFPS(keystate, gameState, targetFPS);
                quit = pair.first;
                localFPS = pair.second;
                timePerFrame = 1000 / localFPS;
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

        // create threads for input and movement handling
        std::thread inputThread(handleInput, std::ref(gameState), keystate);
        std::thread movementThread(handleMovement, std::ref(gameState));

        // wait for threads to finish
        inputThread.join();
        movementThread.join();

        // render the game state
        windowManager.updateWindow(gameState);

        // framerate governing
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