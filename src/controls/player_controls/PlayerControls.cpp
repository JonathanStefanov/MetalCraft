//
// Created by gaspa on 28/12/2022.
//

#include "PlayerControls.h"
#include "../../objects/dropped_item/DroppedItemManager.h"
#include "../../objects/player/Player.h"
#include <iostream>

PlayerControls::PlayerControls(IGameObject *player, Camera &camera, World &world, DroppedItemManager& droppedItemManager)
        : player(player), world(world), camera(camera), droppedItemManager(droppedItemManager) {}

float PlayerControls::getMiningDuration(TextureType textureType) const {
    switch (textureType) {
        case TextureType::LEAF: return 0.25f;
        case TextureType::DIRT:
        case TextureType::GRASS: return 0.55f;
        case TextureType::GLOW_STONE: return 0.8f;
        case TextureType::WOOD: return 1.2f;
        default: return 0.7f;
    }
}

void PlayerControls::resetMining() {
    isMining = false;
    miningStartTime = 0;
    miningDuration = 0;
}

void PlayerControls::moveWithCollision(glm::vec3 oldPosition, glm::vec3 desiredPosition) {
    Transform* transform = player->getTransform();

    auto isGrounded = [this]() {
        return player->physicsData.velocity == 0 && player->physicsData.acceleration == 0;
    };

    bool didStepUp = false;
    auto tryStepUp = [&]() {
        if (!isGrounded() || didStepUp) {
            return false;
        }

        glm::vec3 collisionPosition = transform->position;
        glm::vec3 steppedPosition = transform->position + glm::vec3(0.0f, 1.0f, 0.0f);
        transform->setPosition(steppedPosition.x, steppedPosition.y, steppedPosition.z);
        if (!world.collides(player)) {
            player->physicsData.velocity = 0;
            player->physicsData.acceleration = 0;
            didStepUp = true;
            return true;
        }

        transform->setPosition(collisionPosition.x, collisionPosition.y, collisionPosition.z);
        return false;
    };

    transform->setPosition(desiredPosition.x, oldPosition.y, oldPosition.z);
    if (world.collides(player) && !tryStepUp()) {
        transform->setPosition(oldPosition.x, transform->position.y, oldPosition.z);
    }

    transform->setPosition(transform->position.x, transform->position.y, desiredPosition.z);
    if (world.collides(player) && !tryStepUp()) {
        transform->setPosition(transform->position.x, transform->position.y, oldPosition.z);
    }
}

void PlayerControls::processEvents(GLFWwindow *window, Shader &shader) {
    processMouse(window);

    glm::vec3 oldPosition = player->getTransform()->position;
    glm::vec3 desiredPosition = oldPosition;

    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) != GLFW_PRESS) {
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            player->getTransform()->setPosition(desiredPosition.x, desiredPosition.y, desiredPosition.z);
            player->getTransform()->translate(-speed, 0, 0);
            desiredPosition = player->getTransform()->position;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            player->getTransform()->setPosition(desiredPosition.x, desiredPosition.y, desiredPosition.z);
            player->getTransform()->translate(speed, 0, 0);
            desiredPosition = player->getTransform()->position;
        }
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            player->getTransform()->setPosition(desiredPosition.x, desiredPosition.y, desiredPosition.z);
            player->getTransform()->translate(0, 0, -speed);
            desiredPosition = player->getTransform()->position;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            player->getTransform()->setPosition(desiredPosition.x, desiredPosition.y, desiredPosition.z);
            player->getTransform()->translate(0, 0, speed);
            desiredPosition = player->getTransform()->position;
        }
        player->getTransform()->setPosition(oldPosition.x, oldPosition.y, oldPosition.z);
        moveWithCollision(oldPosition, desiredPosition);

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
                    TextureType blockTexture;
                    if (world.getBlockTextureTypeAt(hitBlock, blockTexture)) {
                        if (!isMining || hitBlock != miningBlock) {
                            isMining = true;
                            miningBlock = hitBlock;
                            miningStartTime = glfwGetTime();
                        }
                        miningDuration = getMiningDuration(blockTexture);

                        if (glfwGetTime() - miningStartTime >= miningDuration) {
                            Item droppedItem = world.removeBlock(hitBlock, &shader);
                            droppedItemManager.spawn(droppedItem, hitBlock + glm::vec3(0.5f, 0.7f, 0.5f), shader);
                            std::cout << "Block removed at " << hitBlock.x << " " << hitBlock.z << " " << hitBlock.y << std::endl;
                            resetMining();
                        }
                    } else {
                        resetMining();
                    }
                } else {
                    resetMining();
                }
            } else {
                resetMining();
            }
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
            resetMining();
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
                            if (world.addBlock(previousEmptyBlock, shader, selected.getTextureType())) {
                                pObj->inventory.consumeSelected();
                            }
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

bool PlayerControls::hasMiningProgress() const {
    return isMining && miningDuration > 0;
}

glm::vec3 PlayerControls::getMiningBlock() const {
    return miningBlock;
}

float PlayerControls::getMiningProgress() const {
    if (!hasMiningProgress()) {
        return 0.0f;
    }

    float progress = (float) ((glfwGetTime() - miningStartTime) / miningDuration);
    if (progress < 0.0f) return 0.0f;
    if (progress > 1.0f) return 1.0f;
    return progress;
}
