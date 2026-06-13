//
// Created by gaspa on 18/01/2023.
//

#include "Player.h"
#include "../mesh/manager/MeshManager.h"
#include "../../texture/manager/TextureManager.h"

Player::Player() {
    player = new GameObject(MeshManager::getMesh(BODY_MESH));
    leftArm = new GameObject(MeshManager::getMesh(LEFTARM_MESH), player->transform);
    rightArm = new GameObject(MeshManager::getMesh(RIGHTARM_MESH), player->transform);
    leftLeg = new GameObject(MeshManager::getMesh(LEFTLEG_MESH), player->transform);
    rightLeg = new GameObject(MeshManager::getMesh(RIGHTLEG_MESH), player->transform);

    leftLeg->transform.rotationAxis = glm::vec3(0, 1, 0);
    rightLeg->transform.rotationAxis = glm::vec3(0, 1, 0);

    rightArm->transform.rotationAxis = glm::vec3(0.37, 1.305, 0);
    rightArm->transform.rotateZ(-90);

    leftArm->transform.rotationAxis = glm::vec3(-0.37, 1.305, 0);
    leftArm->transform.rotateZ(90);


    player->setTexture(TextureManager::getTextureID(PLAYER));
    leftArm->setTexture(TextureManager::getTextureID(PLAYER));
    rightArm->setTexture(TextureManager::getTextureID(PLAYER));
    leftLeg->setTexture(TextureManager::getTextureID(PLAYER));
    rightLeg->setTexture(TextureManager::getTextureID(PLAYER));

    inventory.add(Item(ItemType::WOOD, 64));
    inventory.add(Item(ItemType::DIRT, 64));
    inventory.add(Item(ItemType::GLOW_STONE, 64));
    inventory.add(Item(ItemType::LEAF, 64));
    inventory.add(Item(ItemType::GRASS, 64));
}

void Player::makeObject(Shader &shader) {
    player->makeObject(shader);
    leftArm->makeObject(shader);
    rightArm->makeObject(shader);
    leftLeg->makeObject(shader);
    rightLeg->makeObject(shader);
}

void Player::draw(Shader &shader) {
    if (!isFirstPerson) {
        player->draw(shader);
        leftArm->draw(shader);
        leftLeg->draw(shader);
        rightLeg->draw(shader);
    }

    // In first person, we want to draw the right arm in a fixed location on screen.
    // For now, we'll draw it normally, but we can override its transform here if needed.
    // However, since we need pitch, it's better to render the hand via Minecraft.cpp.
    // We will just skip rightArm->draw(shader) here if in first person and draw it later!
    if (!isFirstPerson) {
        rightArm->draw(shader);
    }

    float newPosValue =  player->transform.getPosition().x + player->transform.getPosition().z;


    leftArm->transform.rotateZ(-physicsData.velocity*100);
    rightArm->transform.rotateZ(physicsData.velocity*100);

    if (leftArm->transform.rotation.z > 90) {
        leftArm->transform.rotation.z = 90;
    }

    if (leftArm->transform.rotation.z < 0) {
        leftArm->transform.rotation.z = 0;
    }

    if(rightArm->transform.rotation.z < -90) {
        rightArm->transform.rotation.z = -90;
    }

    if (rightArm->transform.rotation.z > 0) {
        rightArm->transform.rotation.z = 0;
    }

    if (physicsData.velocity == 0 && physicsData.acceleration == 0) {
        leftArm->transform.rotation.z = 90;
        leftArm->transform.markAsDirtyState();
        rightArm->transform.rotation.z = -90;
        rightArm->transform.markAsDirtyState();
    }


    if (newPosValue != lastPosValue) {
        lastPosValue = newPosValue;

        if (leftLeg->transform.rotation.x > 20) {
            rotationDirection = -1;
        } else if (leftLeg->transform.rotation.x < -20) {
            rotationDirection = 1;
        }

        leftLeg->transform.rotateX( rotationDirection*10);
        rightLeg->transform.rotateX(-rotationDirection*10);
        delta = 10;
    } else {
        delta -= 1;

        if (delta <= 0) {
            leftLeg->transform.setRotationX(0);
            rightLeg->transform.setRotationX(0);
            delta = 0;
        }
    }
}

void Player::drawHand(Shader &shader) {
    if (isFirstPerson) {
        // Save current transform
        glm::vec3 oldPos = rightArm->transform.position;
        glm::vec3 oldRot = rightArm->transform.rotation;
        Transform* oldParent = rightArm->transform.parent;

        // Detach from player body
        rightArm->transform.parent = nullptr;
        
        // Position relative to screen (since view matrix is identity)
        rightArm->transform.position = glm::vec3(1.5f, -1.5f, -3.0f);
        
        // Base rotation to make it look like an arm holding out
        rightArm->transform.rotation = glm::vec3(-30.0f, -20.0f, -20.0f);
        
        // Scale it up
        rightArm->transform.setScale(1.0f, 1.0f, 1.0f);

        // Optional: Add walking bobbing
        if (physicsData.velocity != 0 || physicsData.acceleration != 0) {
            float bob = sin(lastPosValue * 0.5f) * 0.2f;
            rightArm->transform.position.y += bob;
        }

        rightArm->transform.markAsDirtyState();
        rightArm->draw(shader);

        // Restore
        rightArm->transform.parent = oldParent;
        rightArm->transform.position = oldPos;
        rightArm->transform.rotation = oldRot;
        rightArm->transform.setScale(1.0f, 1.0f, 1.0f);
        rightArm->transform.markAsDirtyState();
    }
}

Transform* Player::getTransform()  {
    return &player->transform;
}
