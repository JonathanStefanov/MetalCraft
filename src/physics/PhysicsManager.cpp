//
// Created by gaspa on 02/01/2023.
//

#include "PhysicsManager.h"

void PhysicsManager::update() {
    for (auto &data: objects) {
        PhysicsData &physicsData = data->physicsData;
        float previousY = data->getTransform()->position.y;

        physicsData.velocity += physicsData.acceleration;
        physicsData.velocity -= physicsData.velocity * physicsData.dragCoefficient;

        data->getTransform()->translatePure(0, physicsData.velocity, 0);

        float currentY = data->getTransform()->position.y;
        float standingTopY = 0.0f;
        if (physicsData.velocity <= 0 && world->findStandingBlockTop(data, previousY, currentY, standingTopY)) {
            data->getTransform()->setPosition(data->getTransform()->position.x, standingTopY,
                                             data->getTransform()->position.z);
            physicsData.velocity = 0;
            physicsData.acceleration = 0;
        } else {
            physicsData.acceleration = -0.02;
        }
    }
}

void PhysicsManager::linkGameObject(IGameObject *gameObject) {
    objects.push_back(gameObject);
}

PhysicsManager::PhysicsManager(World *world) : world(world) {}
