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
    // Get position and size of sprite1
    double x1 = sprite1.getPosition()->getX();
    double y1 = sprite1.getPosition()->getY();
    double w1 = sprite1.getPosition()->getWidth();
    double h1 = sprite1.getPosition()->getHeight();

    // Get position and size of sprite2
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
    // Initialize peer-to-peer network, peer2 is client
    PeerNetwork peer2(false);  // Peer 2 is the client
    peer2.init(5555, "localhost");

    // Game Initialization
    WindowManager windowManager;
    GameWorld gameWorld(800, 600);
    PhysicsEngine physicsEngine;
    Timeline globalTimeline(nullptr, 1);
    GameState gameState(globalTimeline, gameWorld, physicsEngine, windowManager);
    const double GRAVITY = 0.5;
    double velocityY = 0.0;
    bool isJumping = false;
    bool onGround = false;
    const double JUMP_FORCE = -20.0;

    windowManager.init(800, 600);

    // Initialize ground sprite
    SDL_Color green = { 0, 255, 0, 255 };
    PositionComponent* groundPos = new StandardPosition(0.0, static_cast<double>(gameWorld.getWorldHeight() - 50), gameWorld.getWorldWidth(), 50);
    DrawerComponent* groundDrawer = new SimpleRectDrawer(green);
    NamedSprite* ground = new NamedSprite("ground", groundPos, nullptr, nullptr, groundDrawer, gameState.getTimeline());
    gameWorld.addObject(ground);

    // Platform (position received from peer 1)
    PositionComponent* platformPos = new StandardPosition(500.0, 200.0, 80, 60);
    DrawerComponent* platformDrawer = new SimpleRectDrawer(green);
    NamedSprite* platform = new NamedSprite("platform", platformPos, nullptr, nullptr, platformDrawer, gameState.getTimeline());
    gameWorld.addObject(platform);

    // Player 2 (local)
    SDL_Color cyan = { 0, 255, 255, 255 };
    PositionComponent* player2Pos = new StandardPosition(200, 200, 50, 50);
    DrawerComponent* player2Drawer = new SimpleRectDrawer(cyan);
    NamedSprite* player2 = new NamedSprite("player2", player2Pos, nullptr, nullptr, player2Drawer, gameState.getTimeline());
    gameWorld.addObject(player2);

    // Player 1 (remote)
    SDL_Color red = { 255, 0, 0, 255 };
    PositionComponent* player1Pos = new StandardPosition(100, 100, 50, 50);
    DrawerComponent* player1Drawer = new SimpleRectDrawer(red);
    NamedSprite* player1 = new NamedSprite("player1", player1Pos, nullptr, nullptr, player1Drawer, gameState.getTimeline());
    gameWorld.addObject(player1);

    Position myPosition = { 200, 200 };
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

        // Handle input for player 2
        const Uint8* currentKeyStates = SDL_GetKeyboardState(nullptr);
        if (currentKeyStates[SDL_SCANCODE_A]) {
            myPosition.x -= 5;
        }
        if (currentKeyStates[SDL_SCANCODE_D]) {
            myPosition.x += 5;
        }

        // Handle jumping
        if (currentKeyStates[SDL_SCANCODE_W] && onGround) {
            velocityY = JUMP_FORCE; // Apply jump force
            isJumping = true;
            onGround = false;
        }

        // Apply gravity
        velocityY += GRAVITY;

        // Update player position with gravity
        myPosition.y += static_cast<int>(velocityY);

        // Ground collision detection for Player 2 using PhysicsEngine
        if (physicsEngine.checkCollisions(*player2, gameState)) {
            if (player2Pos->getY() + player2Pos->getHeight() > groundPos->getY()) {
                player2Pos->setY(groundPos->getY() - player2Pos->getHeight());
                myPosition.y = player2Pos->getY(); // Sync position
                velocityY = 0.0;  // Stop vertical movement
                isJumping = false;
                onGround = true;  // Player is on the ground
            }
        }
        else {
            onGround = false;
        }

        // Platform collision detection for Peer2
        if (physicsEngine.checkCollisions(*player2, gameState)) {
            double playerBottom = player2Pos->getY() + player2Pos->getHeight();
            double platformTop = platformPos->getY();
            double platformBottom = platformPos->getY() + platformPos->getHeight();

            // Check if player is landing on top of the platform
            if (playerBottom > platformTop && player2Pos->getY() < platformTop && velocityY > 0) {
                // Player is landing on the platform
                player2Pos->setY(platformTop - player2Pos->getHeight());  // Place player on top of the platform
                myPosition.y = player2Pos->getY();  // Sync position
                velocityY = 0.0;  // Stop vertical movement
                onGround = true;  // Player is on a platform
            }
            else if (player2Pos->getY() < platformBottom && playerBottom > platformTop) {
                // Prevent the player from passing through the platform from below or sides
                velocityY = 0.0;  // Stop vertical movement if passing through
            }
        }

        // Update player 2's position
        player2Pos->setX(myPosition.x);
        player2Pos->setY(myPosition.y);

        // Send player 2's position to peer 1
        if (myPosition.x != lastSentPosition.x || myPosition.y != lastSentPosition.y) {
            std::string positionMessage = "peer2:" + myPosition.toString();
            peer2.sendMessage(positionMessage);
            std::cout << "Peer2 Sent: " << positionMessage << std::endl;
            lastSentPosition = myPosition;
        }
        // Receive player 1's position and platform position from peer 1
        std::string receivedMessage = peer2.receiveMessage();
        if (!receivedMessage.empty()) {
            // Split the received message into player1 data and platform data
            std::string player1Data = receivedMessage.substr(0, receivedMessage.find(';'));
            std::string platformData = receivedMessage.substr(receivedMessage.find(';') + 1);

            // Update player 1's position
            auto delimPos = player1Data.find(':');
            Position newPos = parsePosition(player1Data.substr(delimPos + 1));
            player1Pos->setX(newPos.x);
            player1Pos->setY(newPos.y);
            std::cout << "Peer2 Received Player1 Position: " << newPos.x << ", " << newPos.y << std::endl;

            // Update platform position
            delimPos = platformData.find(':');
            Position platformNewPos = parsePosition(platformData.substr(delimPos + 1));
            platformPos->setX(platformNewPos.x);
            platformPos->setY(platformNewPos.y);
            std::cout << "Peer2 Received Platform Position: " << platformNewPos.x << ", " << platformNewPos.y << std::endl;
        }

        // Check for collision between player1 and player2 using the custom collision function
        if (checkPeerCollision(*player1, *player2)) {
            std::cout << "Collision detected between Player1 and Player2!" << std::endl;

            // Adjust positions to move players apart after collision
            if (player1->getPosition()->getX() < player2->getPosition()->getX()) {
                player1->getPosition()->setX(player1->getPosition()->getX() - 10);  // Move player1 to the left
                player2->getPosition()->setX(player2->getPosition()->getX() + 10);  // Move player2 to the right
            }
            else {
                player1->getPosition()->setX(player1->getPosition()->getX() + 10);  // Move player1 to the right
                player2->getPosition()->setX(player2->getPosition()->getX() - 10);  // Move player2 to the left
            }

            // Sync the updated positions
            myPosition.x = player2Pos->getX();
            myPosition.y = player2Pos->getY();
            std::string positionMessage = "peer2:" + myPosition.toString();
            peer2.sendMessage(positionMessage);
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
    // Close the window and clean up
    windowManager.close();
    return 0;
}