#include "pch.h"
#include "NamedSprite.h"

NamedSprite::NamedSprite(const std::string& name, size_t typeHash, PositionComponent* position, MovementComponent* movement,
    InputComponent* input, DrawerComponent* drawer, Timeline& timeline)
    : name(name), typeHash(typeHash), position(position), movement(movement), input(input), drawer(drawer), localTimeline(timeline) {}

NamedSprite::~NamedSprite() {
    delete position;
    delete movement;
    delete input;
    delete drawer;
}

std::string NamedSprite::getName() const {
    return name;
}

size_t NamedSprite::getTypeHash() const {
    return typeHash;
}

Timeline& NamedSprite::getTimeline() {
    return localTimeline;
}

PositionComponent* NamedSprite::getPosition() const {
    return position;
}

MovementComponent* NamedSprite::getMovement() const {
    return movement;
}

InputComponent* NamedSprite::getInput() const {
    return input;
}

DrawerComponent* NamedSprite::getDrawer() const {
    return drawer;
}