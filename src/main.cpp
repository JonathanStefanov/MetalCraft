#include <iostream>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "utils/metal/MetalContext.h"
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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
#include "ui/MenuManager.h"
#include "ui/HUDManager.h"
#include "game/GameState.h"
#include "objects/player/Player.h"

GameState currentState = GameState::MENU;
MenuManager* g_menuManager = nullptr;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (currentState != GameState::PLAYING && g_menuManager) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        g_menuManager->handleMouseClicked(xpos, ypos, button, action);
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (currentState != GameState::PLAYING && g_menuManager) {
        g_menuManager->handleMouseMoved(xpos, ypos);
    }
}

const int INITIAL_WINDOW_WIDTH = 1280;
const int INITIAL_WINDOW_HEIGHT = 720;

Shader loadShader(const std::string &vertexPath, const std::string &fragmentPath, bool withTexture = true, bool withLight = true);

void setupMeshs();
void setupTextures();
CubeMap *loadCubeMap();

void renderShadowMap(Minecraft *minecraft, Shader &shadowShader, MTL::Texture* shadowMapTexture, MTL::CommandBuffer* cmdBuf, const glm::mat4 &lightV, const glm::mat4 &lightP);

void renderMainPass(MTL::Texture* targetTexture, MTL::CommandBuffer* cmdBuf, Minecraft *minecraft, Shader &shader, Shader &cubeMapShader, const CubeMap *cubeMap, MTL::Texture* shadowMapTexture, const glm::mat4 &lightSpaceMatrix, int width, int height);


int main() {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialise GLFW \n");
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Don't create an OpenGL context

    GLFWwindow *window = glfwCreateWindow(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT, "MetalCraft", nullptr, nullptr);

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
    Shader uiShader = loadShader("uiVertex", "uiFragment", true, false);
    Shader blurShader = loadShader("uiVertex", "blurFragment", true, false);

    minecraft->linkShader(shader);
    minecraft->linkShader(shadowShader);

    CubeMap *cubeMap = loadCubeMap();
    cubeMap->makeObject(shader);

    glfwSwapInterval(1);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    
    MenuManager menuManager;
    g_menuManager = &menuManager;
    menuManager.init(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT, window);

    HUDManager hudManager;

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
    MTL::Texture* mainColorTexture = nullptr;
    
    double lastTime = glfwGetTime();
    const double targetFrameTime = 1.0 / 60.0;

    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        if (currentTime - lastTime < targetFrameTime) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        lastTime = currentTime;
        
        glfwPollEvents();

        if (currentState == GameState::PLAYING) {
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                currentState = GameState::PAUSED;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
                minecraft->processEvents(window, shader);
                minecraft->updateManagers();
            }
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        glfwGetFramebufferSize(window, &width, &height);
        MetalContext::get()->getMetalLayer()->setDrawableSize(CGSizeMake(width, height));
        
        CA::MetalDrawable* drawable = MetalContext::get()->getMetalLayer()->nextDrawable();
        if (!drawable) {
            continue;
        }

        MTL::CommandBuffer* cmdBuf = MetalContext::get()->getCommandQueue()->commandBuffer();

        if (currentState == GameState::PLAYING) {
            renderShadowMap(minecraft, shadowShader, shadowMapTexture, cmdBuf, lightV, lightP);
            renderMainPass(drawable->texture(), cmdBuf, minecraft, shader, cubeMapShader, cubeMap, shadowMapTexture, lightSpaceMatrix, width, height);

            // Draw HUD over main pass
            MTL::RenderPassDescriptor* uiPass = MTL::RenderPassDescriptor::alloc()->init();
            uiPass->colorAttachments()->object(0)->setTexture(drawable->texture());
            uiPass->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionLoad);
            uiPass->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
            
            MTL::RenderCommandEncoder* uiEncoder = cmdBuf->renderCommandEncoder(uiPass);
            uiEncoder->setRenderPipelineState(uiShader.pipelineState);
            uiEncoder->setDepthStencilState(uiShader.depthStencilState);
            uiShader.encoder = uiEncoder;
            
            int winW, winH;
            glfwGetWindowSize(window, &winW, &winH);
            uiShader.uiProjection = glm::ortho(0.0f, (float)winW, (float)winH, 0.0f, -1.0f, 1.0f);
            
            // Need player's inventory
            auto* playerObj = dynamic_cast<Player*>(minecraft->player);
            if (playerObj) {
                hudManager.draw(uiShader, winW, winH, playerObj->inventory);
            }
            
            uiEncoder->endEncoding();
            uiPass->release();

        } else {
            if (!mainColorTexture || mainColorTexture->width() != width || mainColorTexture->height() != height) {
                if (mainColorTexture) mainColorTexture->release();
                MTL::TextureDescriptor* colorDesc = MTL::TextureDescriptor::alloc()->init();
                colorDesc->setPixelFormat(MetalContext::get()->getMetalLayer()->pixelFormat());
                colorDesc->setWidth(width);
                colorDesc->setHeight(height);
                colorDesc->setUsage(MTL::TextureUsageRenderTarget | MTL::TextureUsageShaderRead);
                mainColorTexture = MetalContext::get()->getDevice()->newTexture(colorDesc);
                colorDesc->release();
            }

            renderShadowMap(minecraft, shadowShader, shadowMapTexture, cmdBuf, lightV, lightP);
            renderMainPass(mainColorTexture, cmdBuf, minecraft, shader, cubeMapShader, cubeMap, shadowMapTexture, lightSpaceMatrix, width, height);

            MTL::RenderPassDescriptor* pass = MTL::RenderPassDescriptor::alloc()->init();
            pass->colorAttachments()->object(0)->setTexture(drawable->texture());
            pass->colorAttachments()->object(0)->setClearColor(MTL::ClearColor::Make(0.1, 0.1, 0.1, 1.0));
            pass->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionClear);
            pass->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
            
            MTL::RenderCommandEncoder* encoder = cmdBuf->renderCommandEncoder(pass);
            encoder->setRenderPipelineState(uiShader.pipelineState);
            encoder->setDepthStencilState(uiShader.depthStencilState);
            uiShader.encoder = encoder;
            
            int winW, winH;
            glfwGetWindowSize(window, &winW, &winH);
            uiShader.uiProjection = glm::ortho(0.0f, (float)winW, (float)winH, 0.0f, -1.0f, 1.0f);
            
            menuManager.draw(uiShader, winW, winH, mainColorTexture, &blurShader);
            
            encoder->endEncoding();
            pass->release();
        }

        cmdBuf->presentDrawable(drawable);
        cmdBuf->commit();
    }

    if (shadowMapTexture) shadowMapTexture->release();
    if (mainColorTexture) mainColorTexture->release();
    MetalContext::cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void renderMainPass(MTL::Texture* targetTexture, MTL::CommandBuffer* cmdBuf, Minecraft *minecraft, Shader &shader, Shader &cubeMapShader, const CubeMap *cubeMap, MTL::Texture* shadowMapTexture, const glm::mat4 &lightSpaceMatrix, int width, int height) {
    MTL::RenderPassDescriptor* pass = MTL::RenderPassDescriptor::alloc()->init();
    pass->colorAttachments()->object(0)->setTexture(targetTexture);
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
    encoder->setFrontFacingWinding(MTL::WindingCounterClockwise);
    encoder->setCullMode(MTL::CullModeBack);
    
    // Draw Skybox
    encoder->setRenderPipelineState(cubeMapShader.pipelineState);
    encoder->setDepthStencilState(cubeMapShader.depthStencilState);
    cubeMapShader.encoder = encoder;
    
    extern GameState currentState;
    static float menuRotation = 0.0f;
    if (currentState == GameState::MENU) {
        menuRotation += 0.005f;
        glm::vec3 cameraPos = glm::vec3(50.0f + 40.0f * cos(menuRotation), 60.0f, 50.0f + 40.0f * sin(menuRotation));
        glm::vec3 cameraTarget = glm::vec3(50.0f, 20.0f, 50.0f);
        glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
        
        // Remove translation for skybox
        glm::mat4 skyView = glm::mat4(glm::mat3(view)); 
        cubeMapShader.setMatrix4("V", skyView);
        cubeMapShader.setMatrix4("P", minecraft->camera->getProjectionMatrix());
    } else {
        minecraft->configureMatrices(cubeMapShader);
    }
    cubeMap->draw(cubeMapShader);
    
    // Draw Minecraft
    encoder->setRenderPipelineState(shader.pipelineState);
    encoder->setDepthStencilState(shader.depthStencilState);
    shader.encoder = encoder;
    shader.setMatrix4("lightSpaceMatrix", lightSpaceMatrix);
    
    if (currentState == GameState::MENU) {
        glm::vec3 cameraPos = glm::vec3(50.0f + 40.0f * cos(menuRotation), 60.0f, 50.0f + 40.0f * sin(menuRotation));
        glm::vec3 cameraTarget = glm::vec3(50.0f, 20.0f, 50.0f);
        glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
        
        shader.setMatrix4("V", view);
        shader.setMatrix4("P", minecraft->camera->getProjectionMatrix());
        shader.setVector3f("u_view_pos", cameraPos);
    } else {
        minecraft->configureMatrices(shader);
        shader.setVector3f("u_view_pos", minecraft->camera->transform.position);
    }
    
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
    encoder->setFrontFacingWinding(MTL::WindingCounterClockwise);
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
