#include <iostream>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "utils/metal/MetalContext.h"
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <glm/glm.hpp>
#include "stb_image.h"

#include <thread>

#include "utils/shader/shader/Shader.h"
#include "objects/game_object/GameObject.h"
#include "utils/shader/shader/Light.h"
#include "objects/camera/Camera.h"
#include "controls/player_controls/PlayerControls.h"
#include "utils/world/generate_world.h"
#include "texture/manager/TextureManager.h"
#include "controls/camera/CameraControls.h"
#include "game/Minecraft.h"
#include "objects/mesh/manager/MeshManager.h"
#include "cubemap/CubeMap.h"

const int INITIAL_WINDOW_WIDTH = 500;
const int INITIAL_WINDOW_HEIGHT = 500;

Shader loadShader(const std::string &vertexPath, const std::string &fragmentPath, bool withTexture = true, bool withLight = true);

void setupMeshs();
void setupTextures();
CubeMap *loadCubeMap();

void renderShadowMap(Minecraft *minecraft, Shader &shadowShader, MTL::Texture* shadowMapTexture, MTL::CommandBuffer* cmdBuf, const glm::mat4 &lightV, const glm::mat4 &lightP);

void renderMainPass(CA::MetalDrawable* drawable, MTL::CommandBuffer* cmdBuf, Minecraft *minecraft, Shader &shader, Shader &cubeMapShader, const CubeMap *cubeMap, MTL::Texture* shadowMapTexture, const glm::mat4 &lightSpaceMatrix, int width, int height);


int main() {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialise GLFW \n");
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Don't create an OpenGL context

    GLFWwindow *window = glfwCreateWindow(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT, "Gaspard is cool", nullptr, nullptr);

    if (window == nullptr) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window\n");
    }

    MetalContext::init(window);

    setupMeshs();
    setupTextures();

    auto *minecraft = new Minecraft(100, 100, 1, 300, 2, glm::vec3(15, 1, 15), window);

    Shader shadowShader = loadShader("shadowVertex", "shadowFragment", false, false);
    Shader shader = loadShader("vertexMain", "fragmentMain");
    Shader cubeMapShader = loadShader("cubemapVertex", "cubemapFragment");

    minecraft->linkShader(shader);
    minecraft->linkShader(shadowShader);

    CubeMap *cubeMap = loadCubeMap();
    cubeMap->makeObject(shader);

    glfwSwapInterval(1);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    // Shadow Map Setup
    int shadowTextureWidth = 4096;
    int shadowTextureHeight = 4096;
    MTL::TextureDescriptor* shadowDesc = MTL::TextureDescriptor::alloc()->init();
    shadowDesc->setPixelFormat(MTL::PixelFormatDepth32Float);
    shadowDesc->setWidth(shadowTextureWidth);
    shadowDesc->setHeight(shadowTextureHeight);
    shadowDesc->setUsage(MTL::TextureUsageRenderTarget | MTL::TextureUsageShaderRead);
    MTL::Texture* shadowMapTexture = MetalContext::get()->getDevice()->newTexture(shadowDesc);
    shadowDesc->release();

    const glm::mat4 &lightV = minecraft->light->getSpaceMatrix();
    const glm::mat4 &lightP = Light::getOrthoProjectionMatrix();
    glm::mat4 lightSpaceMatrix = lightP * lightV;

    int width, height;

    while (!glfwWindowShouldClose(window)) {
        minecraft->processEvents(window, shader);

        minecraft->updateManagers();
        glfwPollEvents();
        minecraft->processEvents(window, shader);
        minecraft->updateManagers();

        glfwGetFramebufferSize(window, &width, &height);
        MetalContext::get()->getMetalLayer()->setDrawableSize(CGSizeMake(width, height));
        
        CA::MetalDrawable* drawable = MetalContext::get()->getMetalLayer()->nextDrawable();
        if (!drawable) {
            continue;
        }

        MTL::CommandBuffer* cmdBuf = MetalContext::get()->getCommandQueue()->commandBuffer();

        renderShadowMap(minecraft, shadowShader, shadowMapTexture, cmdBuf, lightV, lightP);

        renderMainPass(drawable, cmdBuf, minecraft, shader, cubeMapShader, cubeMap, shadowMapTexture, lightSpaceMatrix, width, height);

        cmdBuf->presentDrawable(drawable);
        cmdBuf->commit();
    }

    if (shadowMapTexture) shadowMapTexture->release();
    MetalContext::cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void renderMainPass(CA::MetalDrawable* drawable, MTL::CommandBuffer* cmdBuf, Minecraft *minecraft, Shader &shader, Shader &cubeMapShader, const CubeMap *cubeMap, MTL::Texture* shadowMapTexture, const glm::mat4 &lightSpaceMatrix, int width, int height) {
    MTL::RenderPassDescriptor* pass = MTL::RenderPassDescriptor::alloc()->init();
    pass->colorAttachments()->object(0)->setTexture(drawable->texture());
    pass->colorAttachments()->object(0)->setClearColor(MTL::ClearColor::Make(0.1, 0.1, 0.1, 1.0));
    pass->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionClear);
    pass->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
    
    static MTL::Texture* depthTexture = nullptr;
    if (!depthTexture || depthTexture->width() != width || depthTexture->height() != height) {
        if (depthTexture) depthTexture->release();
        MTL::TextureDescriptor* depthDesc = MTL::TextureDescriptor::alloc()->init();
        depthDesc->setPixelFormat(MTL::PixelFormatDepth32Float);
        depthDesc->setWidth(width);
        depthDesc->setHeight(height);
        depthDesc->setUsage(MTL::TextureUsageRenderTarget);
        depthTexture = MetalContext::get()->getDevice()->newTexture(depthDesc);
        depthDesc->release();
    }
    
    pass->depthAttachment()->setTexture(depthTexture);
    pass->depthAttachment()->setClearDepth(1.0);
    pass->depthAttachment()->setLoadAction(MTL::LoadActionClear);
    pass->depthAttachment()->setStoreAction(MTL::StoreActionDontCare);
    
    MTL::RenderCommandEncoder* encoder = cmdBuf->renderCommandEncoder(pass);
    encoder->setCullMode(MTL::CullModeBack);
    
    // Draw Skybox
    encoder->setRenderPipelineState(cubeMapShader.pipelineState);
    encoder->setDepthStencilState(cubeMapShader.depthStencilState);
    cubeMapShader.encoder = encoder;
    minecraft->configureMatrices(cubeMapShader);
    cubeMap->draw(cubeMapShader);
    
    // Draw Minecraft
    encoder->setRenderPipelineState(shader.pipelineState);
    encoder->setDepthStencilState(shader.depthStencilState);
    shader.encoder = encoder;
    shader.setMatrix4("lightSpaceMatrix", lightSpaceMatrix);
    minecraft->configureMatrices(shader);
    shader.setVector3f("u_view_pos", minecraft->camera->transform.position);
    
    encoder->setFragmentTexture(shadowMapTexture, 1);
    encoder->setFragmentTexture(cubeMap->texture, 2); // Set cubemap for water reflection
    minecraft->render(shader);
    
    encoder->endEncoding();
    pass->release();
}

void renderShadowMap(Minecraft *minecraft, Shader &shadowShader, MTL::Texture* shadowMapTexture, MTL::CommandBuffer* cmdBuf, const glm::mat4 &lightV, const glm::mat4 &lightP) {
    MTL::RenderPassDescriptor* pass = MTL::RenderPassDescriptor::alloc()->init();
    pass->depthAttachment()->setTexture(shadowMapTexture);
    pass->depthAttachment()->setClearDepth(1.0);
    pass->depthAttachment()->setLoadAction(MTL::LoadActionClear);
    pass->depthAttachment()->setStoreAction(MTL::StoreActionStore);
    
    MTL::RenderCommandEncoder* encoder = cmdBuf->renderCommandEncoder(pass);
    encoder->setCullMode(MTL::CullModeBack);
    encoder->setRenderPipelineState(shadowShader.pipelineState);
    encoder->setDepthStencilState(shadowShader.depthStencilState);
    
    shadowShader.encoder = encoder;
    shadowShader.setMatrix4("P", lightP);
    shadowShader.setMatrix4("V", lightV);
    
    minecraft->render(shadowShader);
    
    encoder->endEncoding();
    pass->release();
}

CubeMap *loadCubeMap() {
    std::map<std::string, int> facesToLoad = {
            {"posx.jpg", 0},
            {"negx.jpg", 1},
            {"posy.jpg", 2},
            {"negy.jpg", 3},
            {"posz.jpg", 4},
            {"negz.jpg", 5},
    };

    auto *dayCubeMap = new CubeMap("resources/skybox/day/skybox_", facesToLoad);
    return dayCubeMap;
}

void setupTextures() {
    TextureManager::linkTexture(DIRT, "resources/textures/dirt.jpg");
    TextureManager::linkTexture(WOOD, "resources/textures/wood.jpg");
    TextureManager::linkTexture(LEAF, "resources/textures/leaves.jpg");
    TextureManager::linkTexture(PLAYER, "resources/textures/steve.jpg");
    TextureManager::linkTexture(GLOW_STONE, "resources/textures/glowstone.jpg");
    TextureManager::linkTexture(WHITE_SHEEP, "resources/textures/sheep.jpg");
    TextureManager::linkTexture(BROWN_VILLAGER, "resources/textures/villager.jpg");
    TextureManager::linkTexture(GRASS, "resources/textures/grass.jpg");
    TextureManager::linkTexture(WATER, "resources/textures/water.jpg");
}

void setupMeshs() {
    MeshManager::linkMesh(BLOCK, "resources/objects/cube.obj");
    MeshManager::linkMesh(HUMAN, "resources/objects/stevy.obj");
    MeshManager::linkMesh(SHEEP, "resources/objects/sheep/sheep.obj");
    MeshManager::linkMesh(VILLAGER, "resources/objects/villager.obj");
    MeshManager::linkMesh(CUBEMAP, "resources/objects/grass.obj");
    MeshManager::linkMesh(PLANE, "resources/objects/plane.obj");
    MeshManager::linkMesh(BODY_MESH, "resources/objects/steve_body/stevy_no_limbs.obj");
    MeshManager::linkMesh(LEFTARM_MESH, "resources/objects/steve_body/left_arm.obj");
    MeshManager::linkMesh(RIGHTARM_MESH, "resources/objects/steve_body/right_arm.obj");
    MeshManager::linkMesh(LEFTLEG_MESH, "resources/objects/steve_body/left_leg.obj");
    MeshManager::linkMesh(RIGHTLEG_MESH, "resources/objects/steve_body/right_leg.obj");
}

Shader loadShader(const std::string &vertexFunctionName, const std::string &fragmentFunctionName, bool withTexture, bool withLight) {
    return {vertexFunctionName, fragmentFunctionName, withTexture, withLight};
}
