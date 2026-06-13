#pragma once
#include <vector>
#include <memory>
#include "UIElement.h"

class Shader;
class Font;
struct GLFWwindow;

namespace MTL {
    class Texture;
}

class MenuManager {
public:
    MenuManager();
    ~MenuManager();

    void init(int screenWidth, int screenHeight, GLFWwindow* window);
    void draw(Shader& shader, int screenWidth, int screenHeight, MTL::Texture* bgTex, Shader* blurShader);
    void handleMouseMoved(double x, double y);
    void handleMouseClicked(double x, double y, int button, int action);

private:
    std::vector<std::unique_ptr<UIElement>> elements;
    Font* font;
};
