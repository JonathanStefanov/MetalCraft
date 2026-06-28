//
// Created by gaspa on 28/12/2022.
//

#include "PlayerControls.h"
#include "../../objects/player/Player.h"
#include <iostream>

PlayerControls::PlayerControls(IGameObject *player, Camera &camera, World &world)
        : player(player), camera(camera), world(world) {}

void PlayerControls::processEvents(GLFWwindow *window, Shader &shader) {
    processMouse(window);

    glm::vec3 oldPosition = player->getTransform()->position;

    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) != GLFW_PRESS) {
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            player->getTransform()->translate(-speed, 0, 0);
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            player->getTransform()->translate(speed, 0, 0);
        }
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            player->getTransform()->translate(0, 0, -speed);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            player->getTransform()->translate(0, 0, speed);
        }
        // space to jump
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            if (player->physicsData.velocity == 0 && player->physicsData.acceleration == 0) {
                player->physicsData.velocity = 0.2;
            }
        }
        // shift to go down
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            player->getTransform()->translate(0, -1, 0);
        }

        // check if left click
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            if (camera.firstPerson) {
                glm::mat4 rotationMatrixY  = glm::rotate(glm::mat4(1.0), glm::radians(180 + player->getTransform()->rotation.y), glm::vec3(0, 1.0f, 0));
                glm::mat4 rotationMatrixX  = glm::rotate(glm::mat4(1.0), glm::radians(-camera.firstPersonRotation), glm::vec3(1.0f, 0, 0));

                glm::vec3 direction4 = glm::vec3( rotationMatrixY * rotationMatrixX * glm::vec4(0, 0, 1, 1));
                glm::vec3 direction = glm::normalize(direction4);
                
                glm::vec3 currentPoint = player->getTransform()->position + glm::vec3(0, camera.firstPersonDelta.y, 0);

                glm::vec3 hitBlock, previousEmptyBlock;
                if (world.raycastBlocks(currentPoint, direction, 6.0f, hitBlock, previousEmptyBlock)) {
                    world.removeBlock(hitBlock);
                    std::cout << "Block removed at " << hitBlock.x << " " << hitBlock.z << " " << hitBlock.y << std::endl;
                }
            }
        }
        // check if right click
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            didClick = true;
        }


        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE && didClick) {
            didClick = false;
            if (camera.firstPerson) {

                glm::mat4 rotationMatrixY  = glm::rotate(glm::mat4(1.0), glm::radians(180 +player->getTransform()->rotation.y), glm::vec3(0, 1.0f, 0));
                glm::mat4 rotationMatrixX  = glm::rotate(glm::mat4(1.0), glm::radians(-camera.firstPersonRotation), glm::vec3(1.0f, 0, 0));


                glm::vec3 direction4 = glm::vec3( rotationMatrixY*rotationMatrixX * glm::vec4(0, 0, 1, 1));
                glm::vec3 direction = glm::normalize(direction4);

                glm::vec3 currentPoint = player->getTransform()->position + glm::vec3(0, camera.firstPersonDelta.y, 0);

                glm::vec3 hitBlock, previousEmptyBlock;
                if (world.raycastBlocks(currentPoint, direction, 6.0f, hitBlock, previousEmptyBlock)) {
                    Player* pObj = dynamic_cast<Player*>(player);
                    if (pObj) {
                        Item selected = pObj->inventory.getSelectedItem();
                        if (selected.type != ItemType::NONE) {
                            world.addBlock(previousEmptyBlock, shader, selected.getTextureType());
                        }
                    } else {
                        world.addBlock(previousEmptyBlock, shader);
                    }
                }
            }
        }
    }

    // Hotbar selection
    Player* pObj = dynamic_cast<Player*>(player);
    if (pObj) {
        for (int i = 0; i < 9; i++) {
            if (glfwGetKey(window, GLFW_KEY_1 + i) == GLFW_PRESS) {
                pObj->inventory.selectedSlot = i;
            }
        }
    }

    // rotate if pressing left alt
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) {
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            player->getTransform()->rotateY(2);
        } else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            player->getTransform()->rotateY(-2);
        }
    }


    if (world.collides(player)) {
        player->getTransform()->position = oldPosition;
    }
}


void PlayerControls::processMouse(GLFWwindow *window) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    double x, y;

    glfwGetCursorPos(window, &x, &y);

    double delta_x = x - lastX;
    // double delta_y = y - lastY; // strange behaviour (should move camera not player)

    player->getTransform()->rotateY(-delta_x * mouseSensitivity);

    // if escape is pressed, reset the mouse position
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) {
        glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);
    }
    // wrap pointer


    lastX = windowWidth / 2;
}

