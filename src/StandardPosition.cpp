#include "pch.h"
#include "StandardPosition.h"

StandardPosition::StandardPosition(double x, double y, int width, int height)
    : x(x), y(y), width(width), height(height), lastX(x), lastY(y) {}

double StandardPosition::getX() const {
    return x;
}

double StandardPosition::getY() const {
    return y;
}

int StandardPosition::getWidth() const {
    return width;
}

int StandardPosition::getHeight() const {
    return height;
}

void StandardPosition::setX(double newX) {
    std::lock_guard<std::mutex> lock(mtx);
    lastX = x;
    x = newX;
}

void StandardPosition::setY(double newY) {
    std::lock_guard<std::mutex> lock(mtx);
    lastY = y;
    y = newY;
}

void StandardPosition::setWidth(int newWidth) {
    std::lock_guard<std::mutex> lock(mtx);
    width = newWidth;
}

void StandardPosition::setHeight(int newHeight) {
    std::lock_guard<std::mutex> lock(mtx);
    height = newHeight;
}
