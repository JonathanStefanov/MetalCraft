#include "MetalAdapter.h"
#include <QuartzCore/QuartzCore.hpp>

#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

namespace MetalAdapter {
    void setUpMetalLayer(GLFWwindow* window, CA::MetalLayer* metalLayer) {
        NSWindow* nswin = glfwGetCocoaWindow(window);
        NSView* view = [nswin contentView];
        
        // Convert the C++ CA::MetalLayer pointer to an Objective-C CAMetalLayer pointer
        CAMetalLayer* layer = (__bridge CAMetalLayer*)metalLayer;
        
        [view setWantsLayer:YES];
        [view setLayer:layer];
    }
}
