#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include "MetalContext.h"
#include "MetalAdapter.h"

MetalContext* MetalContext::_instance = nullptr;

MetalContext* MetalContext::get() {
    return _instance;
}

void MetalContext::init(GLFWwindow* window) {
    if (!_instance) {
        _instance = new MetalContext(window);
    }
}

void MetalContext::cleanup() {
    if (_instance) {
        delete _instance;
        _instance = nullptr;
    }
}

MetalContext::MetalContext(GLFWwindow* window) {
    _device = MTL::CreateSystemDefaultDevice();
    _commandQueue = _device->newCommandQueue();

    _metalLayer = CA::MetalLayer::layer();
    _metalLayer->setDevice(_device);
    _metalLayer->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
    
    // Pass to Objective-C adapter to attach to the window
    MetalAdapter::setUpMetalLayer(window, _metalLayer);
}

MetalContext::~MetalContext() {
    _metalLayer->release();
    _commandQueue->release();
    _device->release();
}
