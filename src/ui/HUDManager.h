#pragma once
#include "../inventory/Inventory.h"
#include "../utils/shader/shader/Shader.h"

class HUDManager {
public:
    void draw(Shader& shader, int screenWidth, int screenHeight, const Inventory& inventory);
};
