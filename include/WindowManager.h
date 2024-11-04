#pragma once

#include <SDL.h>
#include <iostream>
#include "GameState.h"

class WindowManager {
public:
    WindowManager();
    ~WindowManager();

    bool init(int screen_width = 800, int screen_height = 600);
    void updateWindow(GameState& gameState);
    void updateWindowWithScrolling(GameState& gameState); // new update window function to handle scrolling
    void close();

    bool windowExists() const;

    int getWindowWidth() const;
    int getWindowHeight() const;
    int getLastWindowWidth() const;
    int getLastWindowHeight() const;
    void setWindowSize(int width, int height);
    void updateWindowSize(int width, int height, GameState& gameState);

    SDL_Window* getWindow();
    SDL_Renderer* getRenderer();

    bool getProportionalMode() const;
    void setProportionalMode(bool mode);
    void toggleProportionalMode();

    void setXOffset(double x);
    void setYOffset(double y);
    double getXOffset();
    double getYOffset();

    SDL_Color getBackgroundColor() const;
    void setBackgroundColor(const SDL_Color& color);

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    int windowWidth;
    int windowHeight;
    int lastWindowWidth;
    int lastWindowHeight;
    bool proportionalMode;
    bool m_windowExists;

    double x_offset;
    double y_offset;

    SDL_Color bg_color; // background color
};
