#pragma once
#include "../inventory/Inventory.h"
#include "../utils/shader/shader/Shader.h"

class Font;

class HUDManager {
public:
    HUDManager();
    ~HUDManager();

    void draw(Shader& shader, int screenWidth, int screenHeight, const Inventory& inventory);

private:
    Font* font;
};
