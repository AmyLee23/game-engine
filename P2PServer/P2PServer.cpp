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
#include "PeerNetwork.h"
#include "PositionComponent.h"
#include <SDL.h>
#include <sstream>
#include <iostream>
#include <thread>
#include <chrono>

// Helper function to adjust time scale
double adjustTimeScale(double currentScale, double adjustment, double minScale, double maxScale) {
    double newScale = currentScale + adjustment;
    if (newScale > maxScale) return maxScale;
    if (newScale < minScale) return minScale;
    return newScale;
}
// Structure to hold position data (x, y coordinates)
struct Position {
    int x, y;
    // Convert position to a string representation
    std::string toString() const {
        return std::to_string(x) + "," + std::to_string(y);
    }
};

// Parse a position string in the format "x,y" and return a Position struct
Position parsePosition(const std::string& positionStr) {
    size_t commaPos = positionStr.find(',');
    int x = std::stoi(positionStr.substr(0, commaPos));
    int y = std::stoi(positionStr.substr(commaPos + 1));
    return { x, y };
}
// Check for collision between two NamedSprite objects (peer collision detection)
bool checkPeerCollision(NamedSprite& sprite1, NamedSprite& sprite2) {
    // Get the position and dimensions of the first sprite
    double x1 = sprite1.getPosition()->getX();
    double y1 = sprite1.getPosition()->getY();
    double w1 = sprite1.getPosition()->getWidth();
    double h1 = sprite1.getPosition()->getHeight();

    // Get the position and dimensions of the second sprite
    double x2 = sprite2.getPosition()->getX();
    double y2 = sprite2.getPosition()->getY();
    double w2 = sprite2.getPosition()->getWidth();
    double h2 = sprite2.getPosition()->getHeight();

    // Check for overlap on the x-axis and y-axis
    bool overlapX = x1 < x2 + w2 && x1 + w1 > x2;
    bool overlapY = y1 < y2 + h2 && y1 + h1 > y2;

    // If there is overlap on both axes, the sprites are colliding
    return overlapX && overlapY;
}

int main(int argc, char* argv[]) {
    // Initialize peer-to-peer network, peer1 is the host
    PeerNetwork peer1(true);  // Peer 1 is the host
    peer1.init(5555, "localhost");

    // Game Initialization
    WindowManager windowManager;
    GameWorld gameWorld(800, 600);
    PhysicsEngine physicsEngine;
    Timeline globalTimeline(nullptr, 1);
    GameState gameState(globalTimeline, gameWorld, physicsEngine, windowManager);
    const double GRAVITY = 0.5;  // Gravity constant for physics
    double velocityY = 0.0;  // Vertical velocity for gravity/jumping
    bool isJumping = false;  // Track whether player is jumping
    bool onGround = false;  // Track whether player is on the ground
    const double JUMP_FORCE = -20.0;  // Jump force applied when jumping

    windowManager.init(800, 600); // Initialize SDL window

    // Initialize ground sprite
    SDL_Color green = { 0, 255, 0, 255 };
    PositionComponent* groundPos = new StandardPosition(0.0, static_cast<double>(gameWorld.getWorldHeight() - 50), gameWorld.getWorldWidth(), 50);
    DrawerComponent* groundDrawer = new SimpleRectDrawer(green);
    NamedSprite* ground = new NamedSprite("ground", groundPos, nullptr, nullptr, groundDrawer, gameState.getTimeline());
    gameWorld.addObject(ground);

    // Bouncing platform (host controls this)
    PositionComponent* platformPos = new StandardPosition(500.0, 200.0, 80, 60);
    MovementComponent* platformMovement = new BouncingMovement(1.0 / 100.0, 0);
    DrawerComponent* platformDrawer = new SimpleRectDrawer(green);
    NamedSprite* platform = new NamedSprite("platform", platformPos, platformMovement, nullptr, platformDrawer, gameState.getTimeline());
    gameWorld.addObject(platform);

    // Player 1 (local to peer 1)
    SDL_Color red = { 255, 0, 0, 255 };
    PositionComponent* player1Pos = new StandardPosition(100, 100, 50, 50);
    DrawerComponent* player1Drawer = new SimpleRectDrawer(red);
    NamedSprite* player1 = new NamedSprite("player1", player1Pos, nullptr, nullptr, player1Drawer, gameState.getTimeline());
    gameWorld.addObject(player1);

    // Player 2 (remote)
    SDL_Color cyan = { 0, 255, 255, 255 };
    PositionComponent* player2Pos = new StandardPosition(200, 200, 50, 50);
    DrawerComponent* player2Drawer = new SimpleRectDrawer(cyan);
    NamedSprite* player2 = new NamedSprite("player2", player2Pos, nullptr, nullptr, player2Drawer, gameState.getTimeline());
    gameWorld.addObject(player2);

    Position myPosition = { 100, 100 };
    Position lastSentPosition = myPosition;

    bool quit = false;
    SDL_Event e;

    // Set frame rate and frame delay for rendering loop
    const int targetFPS = 60;
    const int frameDelay = 1000 / targetFPS;
    Uint32 frameStart;
    int frameTime;

    while (!quit) {
        int64_t frameStartTime = globalTimeline.getTime();
        frameStart = SDL_GetTicks();

        // Handle events
        while (SDL_PollEvent(&e) != 0) {
            // Handle quitting
            if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_q)) {
                quit = true;
            }

            // Handle window resizing
            if (e.type == SDL_WINDOWEVENT) {
                if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    int windowWidth = e.window.data1;
                    int windowHeight = e.window.data2;
                    windowManager.updateWindowSize(windowWidth, windowHeight, gameState);
                }
            }

            // Handle scaling shapes and pausing
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_s) {
                    gameState.getWindowManager().toggleProportionalMode();
                }
                else if (e.key.keysym.sym == SDLK_p) {
                    if (globalTimeline.isPaused()) {
                        globalTimeline.unpause();
                    }
                    else {
                        globalTimeline.pause();
                    }
                }
            }

            // Handle time scaling
            if (e.type == SDL_KEYDOWN) {
                double adjustment = 0.0;
                if (e.key.keysym.sym == SDLK_EQUALS || e.key.keysym.sym == SDLK_PLUS) {
                    adjustment = 0.1;
                }
                else if (e.key.keysym.sym == SDLK_MINUS) {
                    adjustment = -0.1;
                }
                if (adjustment != 0.0) {
                    double newGlobalTimeScale = adjustTimeScale(globalTimeline.getTimeScale(), adjustment, 0.5, 2.0);
                    globalTimeline.setTimeScale(newGlobalTimeScale);
                    for (auto& pair : gameWorld.getObjects()) {
                        NamedSprite* sprite = pair.second;
                        Timeline& spriteTimeline = sprite->getTimeline();
                        double newSpriteTimeScale = adjustTimeScale(spriteTimeline.getTimeScale(), adjustment, 0.5, 2.0);
                        spriteTimeline.setTimeScale(newSpriteTimeScale);
                    }
                }
            }
        }

        // Handle player 1 movement input (left and right)
        const Uint8* currentKeyStates = SDL_GetKeyboardState(nullptr);
        if (currentKeyStates[SDL_SCANCODE_LEFT]) {
            myPosition.x -= 5;
        }
        if (currentKeyStates[SDL_SCANCODE_RIGHT]) {
            myPosition.x += 5;
        }

        // Handle jumping
        if (currentKeyStates[SDL_SCANCODE_SPACE] && onGround) {
            velocityY = JUMP_FORCE; // Apply jump force
            isJumping = true;
            onGround = false;
        }

        // Apply gravity
        velocityY += GRAVITY;

        // Update player position with gravity
        myPosition.y += static_cast<int>(velocityY);

        // Collision detection with ground
        if (physicsEngine.checkCollisions(*player1, gameState)) {
            if (player1Pos->getY() + player1Pos->getHeight() > groundPos->getY()) {
                player1Pos->setY(groundPos->getY() - player1Pos->getHeight());
                myPosition.y = player1Pos->getY();  // Sync position
                velocityY = 0.0; // Stop vertical movement
                isJumping = false;
                onGround = true;  // Player is on the ground
            }
        }
        else {
            onGround = false;
        }

        // Platform collision detection
        if (physicsEngine.checkCollisions(*player1, gameState)) {
            double playerBottom = player1Pos->getY() + player1Pos->getHeight();
            double platformTop = platformPos->getY();
            double platformBottom = platformPos->getY() + platformPos->getHeight();

            // Check if player is landing on top of the platform
            if (playerBottom > platformTop && player1Pos->getY() < platformTop && velocityY > 0) {
                // Player is landing on the platform
                player1Pos->setY(platformTop - player1Pos->getHeight());  // Place player on top of the platform
                myPosition.y = player1Pos->getY();  // Sync position
                velocityY = 0.0;  // Stop vertical movement
                onGround = true;  // Player is on a platform
            }
            else if (player1Pos->getY() < platformBottom && playerBottom > platformTop) {
                // Prevent the player from passing through the platform from below or sides
                velocityY = 0.0;  // Stop vertical movement if passing through
            }
        }

        // Update player 1's position
        player1Pos->setX(myPosition.x);
        player1Pos->setY(myPosition.y);

        // Move platform (authoritative on peer 1)
        platformMovement->move(*platform, gameState, globalTimeline.getDt());

        // Send both player 1's position and platform position to peer 2
        std::string positionMessage = "peer1:" + myPosition.toString() + ";platform:" + std::to_string(platformPos->getX()) + "," + std::to_string(platformPos->getY());
        peer1.sendMessage(positionMessage);
        std::cout << "Peer1 Sent: " << positionMessage << std::endl;

        // Receive player 2's position from peer 2
        std::string receivedMessage = peer1.receiveMessage();
        if (!receivedMessage.empty()) {
            // Parse and update player 2's position
            auto delimPos = receivedMessage.find(':');
            Position newPos = parsePosition(receivedMessage.substr(delimPos + 1));
            player2Pos->setX(newPos.x);
            player2Pos->setY(newPos.y);
            std::cout << "Peer1 Received Player2 Position: " << newPos.x << ", " << newPos.y << std::endl;

            // Check for collision between player1 and player2 using custom peer collision
            if (checkPeerCollision(*player1, *player2)) {
                std::cout << "Collision detected between Player1 and Player2!" << std::endl;

                // Adjust both players' positions to move them apart
                if (myPosition.x < player2Pos->getX()) {
                    player1Pos->setX(myPosition.x - 10);  // Move peer1 to the left
                    player2Pos->setX(player2Pos->getX() + 10);  // Move peer2 to the right
                }
                else {
                    player1Pos->setX(myPosition.x + 10);  // Move peer1 to the right
                    player2Pos->setX(player2Pos->getX() - 10);  // Move peer2 to the left
                }

                // Sync updated positions after collision
                myPosition.x = player1Pos->getX();
                myPosition.y = player1Pos->getY();
                std::string positionMessage = "peer1:" + myPosition.toString();
                peer1.sendMessage(positionMessage);
            }
        }

        // Render everything
        windowManager.updateWindow(gameState);

        int64_t frameEndTime = globalTimeline.getTime();
        int64_t frameDuration = frameEndTime - frameStartTime;
        int64_t sleepTime = frameDelay - frameDuration;

        if (sleepTime > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
        }

        // Frame rate control
        frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime) {
            SDL_Delay(frameDelay - frameTime);
        }
    }

    // Clean up and close the window
    windowManager.close();
    return 0;
}