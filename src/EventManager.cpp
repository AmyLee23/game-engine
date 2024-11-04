#include "pch.h" 
#include "EventManager.h"
#include "GameInput.h"
#include "Timeline.h"
#include "Event.h"
#include "NamedSprite.h"
#include <json.hpp> 
#include "GameWorld.h"
#include "GameState.h" 
#include "ControllableJumper.h"
#include <iostream>

EventManager::EventManager(GameState& state, GameInput& input)
    : gameState(state), gameInput(input) {}

void EventManager::registerEvent(const std::shared_ptr<Event>& event) {
    eventQueue.push(event);
}

void EventManager::raiseEvent(const std::shared_ptr<Event>& event) {
    eventQueue.push(event);
}
void EventManager::handleEvents() {
    while (!eventQueue.empty()) {
        auto event = eventQueue.top();
        eventQueue.pop();

        if (event->getType() == "DashLeft") {
            NamedSprite* jumper = gameState.getGameWorld().getObjectByName("jumper");
            if (jumper) {
                auto movement = dynamic_cast<ControllableJumper*>(jumper->getMovement());
                if (movement) {
                    movement->applyDashEffect(3.0, gameInput, true);  // Dash multiplier, gameInput reference, dashingLeft=true
                }
            }
        }
        else if (event->getType() == "DashRight") {
            NamedSprite* jumper = gameState.getGameWorld().getObjectByName("jumper");
            if (jumper) {
                auto movement = dynamic_cast<ControllableJumper*>(jumper->getMovement());
                if (movement) {
                    movement->applyDashEffect(3.0, gameInput, false);  // Dash multiplier, gameInput reference, dashingLeft=false
                }
            }
        }
    }
}


std::string EventManager::serializeEvent(const Event& event) {
    nlohmann::json jsonEvent;
    jsonEvent["type"] = event.getType();
    jsonEvent["timestamp"] = event.getTimestamp();
    jsonEvent["priority"] = event.getPriority();
    return jsonEvent.dump();
}

std::shared_ptr<Event> EventManager::deserializeEvent(const std::string& data) {
    nlohmann::json jsonEvent = nlohmann::json::parse(data);
    auto type = jsonEvent["type"].get<std::string>();
    auto priority = jsonEvent["priority"].get<int>();

    Timeline mainTimeline(nullptr, 1);  // Create a main Timeline instance

    // Create the event using the deserialized data
    return std::make_shared<Event>(mainTimeline, type, priority);
}
