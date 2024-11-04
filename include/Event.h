#pragma once
#include <map>
#include <string>
#include "Timeline.h"

class Event {
public:
	Event(Timeline& timeline, const std::string& type, int priority);
    virtual ~Event() = default;

    double getTimestamp() const;
    int getPriority() const;
    std::string getType() const;
    
private:
    Timeline& timeline;
    int64_t timestamp;
    int priority;
    std::string eventType;
};