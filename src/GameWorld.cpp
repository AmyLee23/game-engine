#include "pch.h"
#include "GameWorld.h"

GameWorld::GameWorld()
    : worldWidth(800), worldHeight(600) {}

GameWorld::GameWorld(int width, int height)
    : worldWidth(width), worldHeight(height) {}

const std::unordered_map<std::string, NamedSprite*>& GameWorld::getObjects() const {
    return namedObjects;
}

NamedSprite* GameWorld::getObjectByName(const std::string& name) {
    auto it = namedObjects.find(name);
    return (it != namedObjects.end()) ? it->second : nullptr;
}

int GameWorld::getWorldWidth() const {
    return worldWidth;
}

int GameWorld::getWorldHeight() const {
    return worldHeight;
}

void GameWorld::setWorldWidth(int width) {
    worldWidth = width;
}

void GameWorld::setWorldHeight(int height) {
    worldHeight = height;
}

void GameWorld::addObject(NamedSprite* sprite) {
    namedObjects[sprite->getName()] = sprite;
}

void GameWorld::removeObject(const std::string& name) {
    auto it = namedObjects.find(name);
    if (it != namedObjects.end()) {
        delete it->second;
        namedObjects.erase(it);
    }
}