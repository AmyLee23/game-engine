#pragma once

#include <unordered_map>
#include <string>
#include "NamedSprite.h"

class GameWorld {
private:
    int worldHeight;
    int worldWidth;
    std::unordered_map<std::string, NamedSprite*> namedObjects; 

public:
    GameWorld();
    GameWorld(int width, int height);

    // TODO: named sprites store their own name, is probably stupid to store a map of name to sprite

    const std::unordered_map<std::string, NamedSprite*>& getObjects() const;
    NamedSprite* getObjectByName(const std::string& name);
    void addObject(NamedSprite* sprite);
    void removeObject(const std::string& name);


    int getWorldWidth() const;
    int getWorldHeight() const;

    void setWorldWidth(int width);
    void setWorldHeight(int height);
};