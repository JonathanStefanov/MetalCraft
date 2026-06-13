#include "MenuManager.h"
#include "Button.h"
#include "Font.h"
#include "../game/GameState.h"
#include <GLFW/glfw3.h>

extern GameState currentState;

MenuManager::MenuManager() : font(nullptr) {}

MenuManager::~MenuManager() {
    delete font;
}

void MenuManager::init(int screenWidth, int screenHeight, GLFWwindow* window) {
    font = new Font("resources/fonts/Minecraft.ttf", 64.0f);

    float btnWidth = 300.0f;
    float btnHeight = 80.0f;
    float startX = (screenWidth - btnWidth) / 2.0f;
    float startY = screenHeight / 2.0f - 100.0f;

    auto playBtn = std::make_unique<Button>("Play", startX, startY, btnWidth, btnHeight, font, [window]() {
        currentState = GameState::PLAYING;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    });

    auto quitBtn = std::make_unique<Button>("Quit", startX, startY + 120.0f, btnWidth, btnHeight, font, [window]() {
        glfwSetWindowShouldClose(window, true);
    });

    elements.push_back(std::move(playBtn));
    elements.push_back(std::move(quitBtn));
}

void MenuManager::draw(Shader& shader) {
    for (auto& el : elements) {
        el->draw(shader);
    }
}

void MenuManager::handleMouseMoved(double x, double y) {
    for (auto& el : elements) {
        el->handleMouseMoved(x, y);
    }
}

void MenuManager::handleMouseClicked(double x, double y, int button, int action) {
    for (auto& el : elements) {
        if (el->handleMouseClicked(x, y, button, action)) {
            break;
        }
    }
}
