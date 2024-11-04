#include "pch.h"
#include "Event.h"
#include <stdexcept>
//#include "Timeline.h"

Event::Event(Timeline& timeline, const std::string& type, int priority) : eventType(type), priority(priority), timeline(timeline) {
	timestamp = timeline.getTime();
}

double Event::getTimestamp() const { return timestamp; }
int Event::getPriority() const { return priority; }
std::string Event::getType() const { return eventType; }