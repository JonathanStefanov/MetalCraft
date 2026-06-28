//
// Created by gaspa on 28/12/2022.
//

#ifndef OPENGLPROJECT_PLAYERCONTROLS_H
#define OPENGLPROJECT_PLAYERCONTROLS_H


#include "../../objects/transform/Transform.h"
#include "../../objects/camera/Camera.h"
#include "../../world/World.h"
#include "GLFW/glfw3.h"

class DroppedItemManager;

class PlayerControls {

    IGameObject* player;
    World& world;
    Camera& camera;
    DroppedItemManager& droppedItemManager;

    float speed = 0.2;
    float mouseSensitivity = 0.05;

    double lastX = 0;
    double lastY = 0;
    bool isMining = false;
    glm::vec3 miningBlock = glm::vec3(0);
    double miningStartTime = 0;
    float miningDuration = 0;

    float getMiningDuration(TextureType textureType) const;
    void resetMining();
    void moveWithCollision(glm::vec3 oldPosition, glm::vec3 desiredPosition);
public:
    PlayerControls(IGameObject* transform, Camera &camera, World& world, DroppedItemManager& droppedItemManager);

    void processEvents(GLFWwindow* window, Shader &shader);

    void processMouse(GLFWwindow *window);

    bool hasMiningProgress() const;

    glm::vec3 getMiningBlock() const;

    float getMiningProgress() const;

    bool didClick = false;
};


#endif //OPENGLPROJECT_PLAYERCONTROLS_H
