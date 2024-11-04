#pragma once
#include <memory>
#include "Event.h"

class EventHandler {
public:
    virtual void onEvent(std::shared_ptr<Event> event) = 0;
    virtual ~EventHandler() = default;
};