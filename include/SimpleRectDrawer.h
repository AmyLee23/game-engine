#pragma once

#include "DrawerComponent.h"
#include <mutex>
#include <SDL.h>
#include <utility>

class SimpleRectDrawer : public DrawerComponent {
private:
    SDL_Color color;
    std::mutex mtx;

public:
    SimpleRectDrawer(SDL_Color color);

    SDL_Color getColor() const;
    void setColor(SDL_Color newColor);

    void draw(NamedSprite& sprite, GameState& gameState, bool scaling) override;
    virtual void drawWithOffset(NamedSprite& sprite, GameState& gameState, bool scaling, double x_offset, double y_offset) override;
    std::pair<double, double> prepareRelativePosition(PositionComponent* position, GameState& gameState) override;
    std::pair<int, int> prepareScaling(PositionComponent* position, GameState& gameState) override;

private:
    void draw(double x, double y, int width, int height, SDL_Renderer* renderer);
};
