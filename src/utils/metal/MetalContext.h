#pragma once

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

struct GLFWwindow;

class MetalContext {
public:
    static MetalContext* get();
    static void init(GLFWwindow* window);
    static void cleanup();

    MTL::Device* getDevice() const { return _device; }
    MTL::CommandQueue* getCommandQueue() const { return _commandQueue; }
    CA::MetalLayer* getMetalLayer() const { return _metalLayer; }

private:
    MetalContext(GLFWwindow* window);
    ~MetalContext();

    static MetalContext* _instance;

    MTL::Device* _device;
    MTL::CommandQueue* _commandQueue;
    CA::MetalLayer* _metalLayer;
};
