#include "pch.h"
#include "WindowManager.h"
#include "DrawerComponent.h"
#include "GameWorld.h"

// constructor
WindowManager::WindowManager() :
    window(nullptr), renderer(nullptr), windowWidth(800), windowHeight(600),
    lastWindowWidth(windowWidth), lastWindowHeight(windowHeight),
    m_windowExists(false), proportionalMode(false), x_offset(0), y_offset(0), bg_color{ 0, 0, 255, 255 } {} // default blue background

// destructor
WindowManager::~WindowManager() {
    close();
}

bool WindowManager::init(int screen_width, int screen_height) {
    windowWidth = screen_width;
    windowHeight = screen_height;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_DisplayMode desktopMode;
    if (SDL_GetDesktopDisplayMode(0, &desktopMode) != 0) {
        std::cerr << "Could not get desktop display mode! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    if (screen_width > desktopMode.w) {
        screen_width = desktopMode.w;
    }
    if (screen_height > desktopMode.h) {
        screen_height = desktopMode.h - 50;
    }

    window = SDL_CreateWindow("SDL Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
    SDL_SetWindowSize(window, screen_width, screen_height);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    m_windowExists = true;
    return true;
}

void WindowManager::updateWindow(GameState& gameState) {
    if (renderer) {
        SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
        SDL_RenderClear(renderer);

        // draw each sprite in the GameWorld
        for (const auto& pair : gameState.getGameWorld().getObjects()) {
            NamedSprite* sprite = pair.second;
            DrawerComponent* drawer = sprite->getDrawer();
            if (drawer) {
                drawer->draw(*sprite, gameState, proportionalMode);
            }
        }
        SDL_RenderPresent(renderer);
    }
}

void WindowManager::updateWindowWithScrolling(GameState& gameState) {
    if (renderer) {
        SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
        SDL_RenderClear(renderer);

        // draw each sprite in the GameWorld
        for (const auto& pair : gameState.getGameWorld().getObjects()) {
            NamedSprite* sprite = pair.second;
            DrawerComponent* drawer = sprite->getDrawer();
            if (drawer) {
                drawer->drawWithOffset(*sprite, gameState, proportionalMode, x_offset, y_offset);
            }
        }
        SDL_RenderPresent(renderer);
    }
}

void WindowManager::close() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    SDL_Quit();
    m_windowExists = false;
}

bool WindowManager::windowExists() const {
    return m_windowExists;
}

int WindowManager::getWindowWidth() const {
    return windowWidth;
}

int WindowManager::getWindowHeight() const {
    return windowHeight;
}

void WindowManager::setWindowSize(int width, int height) {
    lastWindowWidth = windowWidth;
    lastWindowHeight = windowHeight;
    windowWidth = width;
    windowHeight = height;
    SDL_SetWindowSize(window, windowWidth, windowHeight);
}

void WindowManager::updateWindowSize(int width, int height, GameState& gameState) {
    setWindowSize(width, height);
    updateWindow(gameState);
}

SDL_Window* WindowManager::getWindow() {
    return window;
}

SDL_Renderer* WindowManager::getRenderer() {
    return renderer;
}

int WindowManager::getLastWindowWidth() const {
    return lastWindowWidth;
}

int WindowManager::getLastWindowHeight() const {
    return lastWindowHeight;
}

bool WindowManager::getProportionalMode() const {
    return proportionalMode;
}

void WindowManager::setProportionalMode(bool mode) {
    proportionalMode = mode;
}

void WindowManager::toggleProportionalMode() {
    proportionalMode = !proportionalMode;
}

void WindowManager::setXOffset(double new_offset) {
    x_offset = new_offset;
}

void WindowManager::setYOffset(double new_offset) {
    y_offset = new_offset;
}

double WindowManager::getXOffset() {
    return x_offset;
}

double WindowManager::getYOffset() {
    return y_offset;
}

SDL_Color WindowManager::getBackgroundColor() const {
    return bg_color;
}

void WindowManager::setBackgroundColor(const SDL_Color& color) {
    bg_color = color;
}
