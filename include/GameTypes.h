#pragma once

#include <string>
#include <unordered_map>
#include <functional> // for std::hash

class GameTypes {
private:
    std::unordered_map<std::string, size_t> typeMap;

public:
    GameTypes();
    size_t getTypeHash(const std::string& type);
};
