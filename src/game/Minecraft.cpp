//
// Created by gaspa on 31/12/2022.
//

#include "Minecraft.h"
#include "../objects/mesh/manager/MeshManager.h"
#include "../texture/manager/TextureManager.h"
#include "../objects/player/Player.h"
#include <glm/gtc/matrix_transform.hpp>

Minecraft::Minecraft(int width, int height, int depth, int nbrTrees, int nbCircles, glm::vec3 playerSpawn, GLFWwindow *window) : world(
        new World(1337)) {
    pnjManager = new PNJManager(world);
    physicsManager = new PhysicsManager(world);

    auto block = new Player();
    player = block;
    playerSpawn.y = (float) world->getTerrainHeightAt((int) playerSpawn.x, (int) playerSpawn.z) + 1.0f;
    block->player->transform.position = playerSpawn;
    block->player->transform.markAsDirtyState();
    block->collider = Collider{0.5f, 0.5f, 1.0f};

    toRender.push_back(block);
    physicsManager->linkGameObject(block);


    camera = new Camera(block->player->transform);
    camera->firstPerson = true;
    cameraControls = new CameraControls(*camera, window);
    droppedItemManager = new DroppedItemManager();
    blockOutlineRenderer = new BlockOutlineRenderer();
    playerControls = new PlayerControls(block, *camera, *world, *droppedItemManager);
    double size = 0.6;

    for (int i = 0; i < 20; i++) {
        if (i > 10) {
            size = 0.3;
        }
        auto* sheep = new GameObject(MeshManager::getMesh(MeshType::SHEEP));
        sheep->setTexture(TextureManager::getTextureID(TextureType::WHITE_SHEEP));
        sheep->transform.setPosition(10, 1, 10);
        sheep->transform.setScale(size, size, size);
        sheep->collider = Collider{2.0f, 2.0f, 0.5f};
        physicsManager->linkGameObject(sheep);
        pnjManager->addPNJ(sheep, {1, 50, 0.1});
        toRender.push_back(sheep);
    }

    size = 2;
    for (int i = 0; i < 20; i++) {
        if (i > 10) {
            size = 1.5;
        }
        auto* sheep = new GameObject(MeshManager::getMesh(MeshType::VILLAGER));
        sheep->setTexture(TextureManager::getTextureID(TextureType::BROWN_VILLAGER));
        sheep->transform.setPosition(10, 1, 10);
        sheep->transform.setScale(size, size, size);
        sheep->collider = Collider{2.0f, 2.0f, 0.5f};
        physicsManager->linkGameObject(sheep);
        pnjManager->addPNJ(sheep, {1, 50, 0.1});
        toRender.push_back(sheep);
    }





    light = new Light(
            glm::vec3(50, 100, 50),
            glm::vec3(0.0, 0.0, 0.0),
            0.5,
            0, // no specular in minecraft
            1,
            5.0,
            0.0014,
            0.00001,
            1.0
    );


}

void Minecraft::render(Shader &shader) {
    if (shader.withLight) {
        light->linkShader(shader);
    }
    if (shader.withLight) {
        light->use(shader);
    }

    world->draw(shader, player->getTransform()->getPosition());
    player->draw(shader);

    for (auto &gameObject : toRender) {
        gameObject->draw(shader);
    }

    droppedItemManager->draw(shader);

    // Draw the hand in first person, overriding the view matrix to fix it to the screen
    auto* playerObj = dynamic_cast<Player*>(player);
    if (playerObj && playerObj->isFirstPerson) {
        // We temporarily clear the view matrix so the hand is drawn relative to camera
        glm::mat4 oldView = camera->getViewMatrix();
        shader.setMatrix4("V", glm::mat4(1.0f));
        playerObj->drawHand(shader);
        shader.setMatrix4("V", oldView); // Restore
    }
}

void Minecraft::renderTargetBlockOutline(Shader &shader) {
    glm::vec3 targetBlock;
    if (getTargetedBlock(targetBlock)) {
        blockOutlineRenderer->draw(shader, targetBlock);
        if (playerControls->hasMiningProgress() && playerControls->getMiningBlock() == targetBlock) {
            blockOutlineRenderer->drawCracks(shader, targetBlock, playerControls->getMiningProgress());
        }
    }
}

void Minecraft::linkShader(Shader &shader) {
    world->updateLoadedChunks(player->getTransform()->getPosition(), shader);
    world->makeObjects(shader);
    player->makeObject(shader);
    for (auto &gameObject : toRender) {
        gameObject->makeObject(shader);
    }
}

void Minecraft::processEvents(GLFWwindow *window, Shader &shader) {
    cameraControls->processEvents(window);
    playerControls->processEvents(window, shader);
}

void Minecraft::configureMatrices(Shader &shader) const {
    shader.setMatrix4("V", camera->getViewMatrix());
    shader.setMatrix4("P", camera->getProjectionMatrix());
}

void Minecraft::updateManagers(Shader &shader) {
    world->updateLoadedChunks(player->getTransform()->getPosition(), shader);
    pnjManager->update();
    physicsManager->update();
    auto* playerObj = dynamic_cast<Player*>(player);
    if (playerObj) {
        droppedItemManager->update(*playerObj, *world);
    }
}

bool Minecraft::getTargetedBlock(glm::vec3& outBlockPosition) const {
    if (!camera->firstPerson) {
        return false;
    }

    glm::mat4 rotationMatrixY = glm::rotate(glm::mat4(1.0), glm::radians(180 + player->getTransform()->rotation.y), glm::vec3(0, 1.0f, 0));
    glm::mat4 rotationMatrixX = glm::rotate(glm::mat4(1.0), glm::radians(-camera->firstPersonRotation), glm::vec3(1.0f, 0, 0));
    glm::vec3 direction = glm::normalize(glm::vec3(rotationMatrixY * rotationMatrixX * glm::vec4(0, 0, 1, 1)));
    glm::vec3 start = player->getTransform()->position + glm::vec3(0, camera->firstPersonDelta.y, 0);

    glm::vec3 previousEmptyBlock;
    return world->raycastBlocks(start, direction, 6.0f, outBlockPosition, previousEmptyBlock);
}
