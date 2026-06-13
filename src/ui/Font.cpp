#define STB_TRUETYPE_IMPLEMENTATION
#include "Font.h"
#include <iostream>
#include <fstream>
#include "../utils/metal/MetalContext.h"
#include "../utils/shader/shader/Shader.h"
#include <Metal/Metal.hpp>
#include <glm/gtc/type_ptr.hpp>

Font::Font(const std::string& fontPath, float size) : fontSize(size) {
    std::ifstream file(fontPath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cout << "Failed to open font file: " << fontPath << std::endl;
        return;
    }

    std::streamsize sizeInBytes = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<unsigned char> ttfBuffer(sizeInBytes);
    if (!file.read((char*)ttfBuffer.data(), sizeInBytes)) {
        std::cout << "Failed to read font file" << std::endl;
        return;
    }

    int bitmapWidth = 512;
    int bitmapHeight = 512;
    std::vector<unsigned char> tempBitmap(bitmapWidth * bitmapHeight);

    stbtt_BakeFontBitmap(ttfBuffer.data(), 0, fontSize, tempBitmap.data(), bitmapWidth, bitmapHeight, 32, 96, cdata);

    MTL::TextureDescriptor* texDesc = MTL::TextureDescriptor::alloc()->init();
    texDesc->setPixelFormat(MTL::PixelFormatR8Unorm);
    texDesc->setWidth(bitmapWidth);
    texDesc->setHeight(bitmapHeight);
    
    MTL::Device* device = MetalContext::get()->getDevice();
    texture = device->newTexture(texDesc);
    texDesc->release();

    MTL::Region region = MTL::Region::Make2D(0, 0, bitmapWidth, bitmapHeight);
    texture->replaceRegion(region, 0, tempBitmap.data(), bitmapWidth);
}

Font::~Font() {
    if (texture) texture->release();
    if (vertexBuffer) vertexBuffer->release();
}

float Font::getTextWidth(const std::string& text) {
    float width = 0;
    for (char c : text) {
        if (c >= 32 && c < 128) {
            width += cdata[c - 32].xadvance;
        }
    }
    return width;
}

void Font::drawText(Shader& shader, const std::string& text, float startX, float startY, const glm::vec4& color) {
    MTL::RenderCommandEncoder* encoder = shader.encoder;
    if (!encoder) return;

    std::vector<UIVertex> vertices;
    float x = startX;
    float y = startY;

    for (char c : text) {
        if (c >= 32 && c < 128) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(cdata, texture->width(), texture->height(), c - 32, &x, &y, &q, 1);

            // stbtt returns coordinates for OpenGL (Y down). Metal NDC is Y up, but our ortho projection handles it
            vertices.push_back({{q.x0, q.y0}, {q.s0, q.t0}});
            vertices.push_back({{q.x1, q.y1}, {q.s1, q.t1}});
            vertices.push_back({{q.x0, q.y1}, {q.s0, q.t1}});

            vertices.push_back({{q.x0, q.y0}, {q.s0, q.t0}});
            vertices.push_back({{q.x1, q.y0}, {q.s1, q.t0}});
            vertices.push_back({{q.x1, q.y1}, {q.s1, q.t1}});
        }
    }

    if (vertices.empty()) return;

    MTL::Device* device = MetalContext::get()->getDevice();
    if (vertexBuffer) vertexBuffer->release();
    vertexBuffer = device->newBuffer(vertices.data(), vertices.size() * sizeof(UIVertex), MTL::ResourceStorageModeShared);

    encoder->setFragmentTexture(texture, 0);

    struct UIUniforms {
        simd::float4x4 projection;
        simd::float4 color;
    } uniforms;

    memcpy(&uniforms.projection, glm::value_ptr(shader.uiProjection), sizeof(simd::float4x4));
    uniforms.color = simd_make_float4(color.r, color.g, color.b, color.a);

    encoder->setVertexBytes(&uniforms, sizeof(UIUniforms), 1);
    encoder->setFragmentBytes(&uniforms, sizeof(UIUniforms), 1);

    encoder->setVertexBuffer(vertexBuffer, 0, 0);
    encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(vertices.size()));
}
