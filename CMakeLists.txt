#Template for INFOH502
#Heavily inspired from https://github.com/Polytonic/Glitter/blob/master/CMakeLists.txt

#project setup
project(openglproject)

#set the minimal version of cmake and the minimal version of the c++ language
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE ON)

add_definitions(-DGLM_FORCE_DEPTH_ZERO_TO_ONE)

find_library(METAL_FRAMEWORK Metal)
find_library(METALKIT_FRAMEWORK MetalKit)
find_library(COCOA_FRAMEWORK Cocoa)
find_library(QUARTZ_FRAMEWORK QuartzCore)
find_library(FOUNDATION_FRAMEWORK Foundation)

#for GLFW
option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(3rdParty/glfw)



include_directories(3rdParty/metal-cpp/
        3rdParty/glfw/include/
        3rdParty/glm/
        3rdParty/stb/)


add_executable(main src/main.cpp src/objects/transform/Transform.cpp src/objects/transform/Transform.h src/utils/shader/shader/Shader.cpp src/utils/shader/shader/Shader.h src/objects/mesh/Mesh.cpp src/objects/mesh/Mesh.h src/objects/renderer/Renderer.cpp src/objects/renderer/Renderer.h src/utils/shader/shader/Light.h src/objects/camera/Camera.cpp  src/world/World.h src/world/World.cpp src/world/BlockOutlineRenderer.cpp src/world/BlockOutlineRenderer.h src/utils/world/generate_world.h src/utils/world/generate_world.cpp src/controls/player_controls/PlayerControls.cpp src/controls/player_controls/PlayerControls.h src/utils/shader/shader/Light.cpp   src/texture/texture/Texture.h src/texture/texture/Texture.cpp src/texture/manager/TextureManager.cpp src/texture/manager/TextureManager.h src/controls/camera/CameraControls.cpp src/controls/camera/CameraControls.h src/game/Minecraft.cpp src/game/Minecraft.h src/objects/mesh/manager/MeshManager.h src/objects/mesh/manager/MeshManager.cpp src/pnj/PNJManager.cpp src/pnj/PNJManager.h src/pnj/PNJData.h src/objects/physics/PhysicsData.h src/objects/physics/PhysicsData.h src/physics/PhysicsManager.cpp src/physics/PhysicsManager.h src/cubemap/CubeMap.cpp src/cubemap/CubeMap.h src/objects/player/Player.cpp src/objects/player/Player.h src/objects/game_object/GameObject.cpp src/objects/game_object/GameObject.h src/objects/game_object/IGameObject.h src/objects/dropped_item/DroppedItem.cpp src/objects/dropped_item/DroppedItem.h src/objects/dropped_item/DroppedItemManager.cpp src/objects/dropped_item/DroppedItemManager.h src/utils/metal/MetalAdapter.mm src/utils/metal/MetalContext.cpp src/ui/Font.cpp src/ui/Font.h src/ui/Button.cpp src/ui/Button.h src/ui/UIElement.h src/ui/MenuManager.cpp src/ui/MenuManager.h src/ui/UIUtils.cpp src/ui/UIUtils.h src/inventory/Item.h src/inventory/Item.cpp src/inventory/Inventory.h src/inventory/Inventory.cpp src/ui/HUDManager.h src/ui/HUDManager.cpp)
#Specify which libraries you want to use with your executable
target_link_libraries(main PUBLIC glfw 
    ${METAL_FRAMEWORK} 
    ${METALKIT_FRAMEWORK} 
    ${COCOA_FRAMEWORK} 
    ${QUARTZ_FRAMEWORK} 
    ${FOUNDATION_FRAMEWORK})

add_custom_command(
    TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders
    ${CMAKE_CURRENT_BINARY_DIR}/shaders
)

add_custom_command(
    TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources
    ${CMAKE_CURRENT_BINARY_DIR}/resources
)
