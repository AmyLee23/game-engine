#include "pch.h"
#include "GameTypes.h"

GameTypes::GameTypes() {}

size_t GameTypes::getTypeHash(const std::string& type) {
    auto it = typeMap.find(type);
    if (it == typeMap.end()) {
        // If type doesn't exist, create a new hash and add it to the map
        size_t hashValue = std::hash<std::string>{}(type);
        typeMap[type] = hashValue;
        return hashValue;
    }
    // If it exists, return the hash
    return it->second;
}
