#pragma once

#include "PositionComponent.h"
#include "GameState.h"
#include <utility>
class NamedSprite;

class DrawerComponent {
public:
    virtual ~DrawerComponent() = default;

    virtual void draw(NamedSprite& sprite, GameState& gameState, bool scaling) = 0;
    virtual void drawWithOffset(NamedSprite& sprite, GameState& gameState, bool scaling, double x_offset, double y_offset) = 0;
    virtual std::pair<double, double> prepareRelativePosition(PositionComponent* position, GameState& gameState) = 0;
    virtual std::pair<int, int> prepareScaling(PositionComponent* position, GameState& gameState) = 0;
};