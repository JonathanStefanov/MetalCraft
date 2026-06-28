
//
// Created by gaspa on 28/12/2022.
//

#ifndef OPENGLPROJECT_WORLD_H
#define OPENGLPROJECT_WORLD_H


#include <map>
#include "glm/fwd.hpp"
#include "glm/detail/type_mat3x3.hpp"
#include "../objects/game_object/GameObject.h"
#include "../objects/mesh/manager/MeshManager.h"
#include "../texture/manager/TextureManager.h"

class World {
public:
    World(std::map<std::tuple<int, int, int>, std::tuple<int, MeshType, TextureType>> map, int length, int width, int depth);

    std::map<std::tuple<int, int, int>, GameObject*> worldBlockInstances;
public:
    int width{}, length{}, depth{};


    std::map<std::tuple<int, int, int>, std::tuple<int, MeshType, TextureType>> worldBlocks;



    void create();

    float normalizeAngle(float angle);

    // Returns true if a block is hit within maxDistance.
    // outHitBlock: The integer coordinates of the solid block that was hit.
    // outPreviousEmptyBlock: The integer coordinates of the empty space right before the hit (useful for placing blocks).
    bool raycastBlocks(glm::vec3 startPos, glm::vec3 direction, float maxDistance, glm::vec3& outHitBlock, glm::vec3& outPreviousEmptyBlock);

    void draw(Shader& shader);

    void removeBlock(glm::vec3 blockPos);


    void makeObjects(Shader &shader);

    bool collides(IGameObject* object);

    GameObject *getBlockAt(glm::vec3 &vec);

    void draw(Shader &shader, glm::vec3 playerPosition);

    void addBlock(glm::vec3 blockPos, Shader &shader, TextureType textureType = TextureType::DIRT);
};


#endif //OPENGLPROJECT_WORLD_H
