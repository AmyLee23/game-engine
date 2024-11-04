#pragma once

class PositionComponent {
public:
    virtual ~PositionComponent() = default;

    virtual double getX() const = 0;
    virtual double getY() const = 0;
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;

    virtual void setX(double x) = 0;
    virtual void setY(double y) = 0;
    virtual void setWidth(int width) = 0;
    virtual void setHeight(int height) = 0;
};