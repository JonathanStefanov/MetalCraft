#pragma once

// Forward declarations
namespace CA {
    class MetalLayer;
}

struct GLFWwindow;

namespace MetalAdapter {
    void setUpMetalLayer(GLFWwindow* window, CA::MetalLayer* metalLayer);
}
