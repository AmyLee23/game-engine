#pragma once

#include "PositionComponent.h"
#include <mutex>

class StandardPosition : public PositionComponent {
private:
    double x, y;
    int width, height;
    double lastX, lastY;
    std::mutex mtx;

public:
    StandardPosition(double x, double y, int width, int height);

    double getX() const override;
    double getY() const override;
    int getWidth() const override;
    int getHeight() const override;

    void setX(double x) override;
    void setY(double y) override;
    void setWidth(int width) override;
    void setHeight(int height) override;
};
