#include "pch.h"
#include "SimpleRectDrawer.h"
#include "NamedSprite.h"
#include "PositionComponent.h"
#include "GameState.h"
#include "GameWorld.h"
#include "WindowManager.h"

SimpleRectDrawer::SimpleRectDrawer(SDL_Color color) : color(color) {}

SDL_Color SimpleRectDrawer::getColor() const {
    return color;
}

void SimpleRectDrawer::setColor(SDL_Color newColor) {
    color = newColor;
}

// helper draw function that accepts pre-calculated X, Y, width, and height
void SimpleRectDrawer::draw(double x, double y, int width, int height, SDL_Renderer* renderer) {
    SDL_Rect rect = {
        static_cast<int>(x),
        static_cast<int>(y),
        width,
        height
    };
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}
// standard draw
void SimpleRectDrawer::draw(NamedSprite& sprite, GameState& gameState, bool scaling) {
    std::lock_guard<std::mutex> lock(mtx);
    SDL_Renderer* renderer = gameState.getWindowManager().getRenderer();

    PositionComponent* position = sprite.getPosition();

    if (scaling) {
        std::pair<double, double> relativePosition = prepareRelativePosition(position, gameState); // adjust X, Y
        std::pair<int, int> scaledSize = prepareScaling(position, gameState); // adjust Width, Height
        draw(relativePosition.first, relativePosition.second, scaledSize.first, scaledSize.second, renderer);
    }
    else {
        draw(position->getX(), position->getY(), position->getWidth(), position->getHeight(), renderer); // Draw original
    }
}

// draw with offset
void SimpleRectDrawer::drawWithOffset(NamedSprite& sprite, GameState& gameState, bool scaling, double x_offset, double y_offset) {
    std::lock_guard<std::mutex> lock(mtx);
    SDL_Renderer* renderer = gameState.getWindowManager().getRenderer();

    PositionComponent* position = sprite.getPosition();

    if (scaling) {
        std::pair<double, double> relativePosition = prepareRelativePosition(position, gameState); // adjust X, Y
        std::pair<int, int> scaledSize = prepareScaling(position, gameState); // adjust Width, Height
        draw(relativePosition.first, relativePosition.second, scaledSize.first, scaledSize.second, renderer);
    }
    else {
        draw(position->getX() + x_offset, position->getY() + y_offset, position->getWidth(), position->getHeight(), renderer); // Draw original
    }
}

// Calculate the relative X and Y
std::pair<double, double> SimpleRectDrawer::prepareRelativePosition(PositionComponent* position, GameState& gameState) {
    int worldWidth = gameState.getGameWorld().getWorldWidth();
    int worldHeight = gameState.getGameWorld().getWorldHeight();
    int windowWidth = gameState.getWindowManager().getWindowWidth();
    int windowHeight = gameState.getWindowManager().getWindowHeight();

    double relativeX = (position->getX() / static_cast<double>(worldWidth)) * windowWidth;
    double relativeY = (position->getY() / static_cast<double>(worldHeight)) * windowHeight;

    return { relativeX, relativeY };
}

// Calculate relative Width and Height
std::pair<int, int> SimpleRectDrawer::prepareScaling(PositionComponent* position, GameState& gameState) {
    int worldWidth = gameState.getGameWorld().getWorldWidth();
    int worldHeight = gameState.getGameWorld().getWorldHeight();
    int windowWidth = gameState.getWindowManager().getWindowWidth();
    int windowHeight = gameState.getWindowManager().getWindowHeight();

    int scaledWidth = static_cast<int>((static_cast<double>(position->getWidth()) / worldWidth) * windowWidth);
    int scaledHeight = static_cast<int>((static_cast<double>(position->getHeight()) / worldHeight) * windowHeight);

    return { scaledWidth, scaledHeight };
}
