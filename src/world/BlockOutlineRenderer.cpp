#include "BlockOutlineRenderer.h"

#include "../utils/metal/MetalContext.h"
#include <Metal/Metal.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace {
    void addVertex(std::vector<float>& data, float x, float y, float z) {
        data.push_back(x);
        data.push_back(y);
        data.push_back(z);
        data.push_back(0.0f);
        data.push_back(0.0f);
        data.push_back(0.0f);
        data.push_back(0.0f);
        data.push_back(0.0f);
    }

    void addLine(std::vector<float>& data, glm::vec3 a, glm::vec3 b) {
        addVertex(data, a.x, a.y, a.z);
        addVertex(data, b.x, b.y, b.z);
    }

    void addTriangle(std::vector<float>& data, glm::vec3 a, glm::vec3 b, glm::vec3 c) {
        addVertex(data, a.x, a.y, a.z);
        addVertex(data, b.x, b.y, b.z);
        addVertex(data, c.x, c.y, c.z);
    }

    void addQuad(std::vector<float>& data, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d) {
        addTriangle(data, a, b, c);
        addTriangle(data, a, c, d);
    }

    void addFaceCrackStrip(std::vector<float>& data, float u1, float v1, float u2, float v2) {
        const float min = -0.535f;
        const float max = 0.535f;
        const float minY = -0.035f;
        const float maxY = 1.045f;
        const float stripWidth = 0.055f;

        float dx = u2 - u1;
        float dy = v2 - v1;
        float length = std::sqrt(dx * dx + dy * dy);
        if (length <= 0.001f) {
            return;
        }

        float offsetU = -dy / length * stripWidth;
        float offsetV = dx / length * stripWidth;

        float au = u1 + offsetU;
        float av = v1 + offsetV;
        float bu = u2 + offsetU;
        float bv = v2 + offsetV;
        float cu = u2 - offsetU;
        float cv = v2 - offsetV;
        float du = u1 - offsetU;
        float dv = v1 - offsetV;

        auto x = [&](float u) { return min + u * (max - min); };
        auto y = [&](float v) { return minY + v * (maxY - minY); };
        auto z = [&](float u) { return min + u * (max - min); };

        addQuad(data, {x(au), y(av), max}, {x(bu), y(bv), max}, {x(cu), y(cv), max}, {x(du), y(dv), max});
        addQuad(data, {x(bu), y(bv), min}, {x(au), y(av), min}, {x(du), y(dv), min}, {x(cu), y(cv), min});
        addQuad(data, {min, y(av), z(au)}, {min, y(bv), z(bu)}, {min, y(cv), z(cu)}, {min, y(dv), z(du)});
        addQuad(data, {max, y(bv), z(bu)}, {max, y(av), z(au)}, {max, y(dv), z(du)}, {max, y(cv), z(cu)});
        addQuad(data, {x(au), maxY, z(au)}, {x(bu), maxY, z(bu)}, {x(cu), maxY, z(cu)}, {x(du), maxY, z(du)});
    }
}

BlockOutlineRenderer::BlockOutlineRenderer() {
    const float minX = -0.505f;
    const float maxX = 0.505f;
    const float minY = -0.005f;
    const float maxY = 1.015f;
    const float minZ = -0.505f;
    const float maxZ = 0.505f;

    const float vertices[][8] = {
            {minX, minY, minZ, 0, 0, 0, 0, 0}, {maxX, minY, minZ, 0, 0, 0, 0, 0},
            {maxX, minY, minZ, 0, 0, 0, 0, 0}, {maxX, minY, maxZ, 0, 0, 0, 0, 0},
            {maxX, minY, maxZ, 0, 0, 0, 0, 0}, {minX, minY, maxZ, 0, 0, 0, 0, 0},
            {minX, minY, maxZ, 0, 0, 0, 0, 0}, {minX, minY, minZ, 0, 0, 0, 0, 0},

            {minX, maxY, minZ, 0, 0, 0, 0, 0}, {maxX, maxY, minZ, 0, 0, 0, 0, 0},
            {maxX, maxY, minZ, 0, 0, 0, 0, 0}, {maxX, maxY, maxZ, 0, 0, 0, 0, 0},
            {maxX, maxY, maxZ, 0, 0, 0, 0, 0}, {minX, maxY, maxZ, 0, 0, 0, 0, 0},
            {minX, maxY, maxZ, 0, 0, 0, 0, 0}, {minX, maxY, minZ, 0, 0, 0, 0, 0},

            {minX, minY, minZ, 0, 0, 0, 0, 0}, {minX, maxY, minZ, 0, 0, 0, 0, 0},
            {maxX, minY, minZ, 0, 0, 0, 0, 0}, {maxX, maxY, minZ, 0, 0, 0, 0, 0},
            {maxX, minY, maxZ, 0, 0, 0, 0, 0}, {maxX, maxY, maxZ, 0, 0, 0, 0, 0},
            {minX, minY, maxZ, 0, 0, 0, 0, 0}, {minX, maxY, maxZ, 0, 0, 0, 0, 0},
    };

    vertexBuffer = MetalContext::get()->getDevice()->newBuffer(vertices, sizeof(vertices), MTL::ResourceStorageModeShared);

    const float crackSegments[][4] = {
            {0.50f, 0.55f, 0.34f, 0.72f},
            {0.50f, 0.55f, 0.66f, 0.70f},
            {0.34f, 0.72f, 0.25f, 0.50f},
            {0.66f, 0.70f, 0.76f, 0.48f},
            {0.50f, 0.55f, 0.48f, 0.35f},
            {0.48f, 0.35f, 0.30f, 0.23f},
            {0.48f, 0.35f, 0.70f, 0.22f},
            {0.25f, 0.50f, 0.16f, 0.32f},
            {0.76f, 0.48f, 0.88f, 0.36f},
            {0.30f, 0.23f, 0.22f, 0.10f},
            {0.70f, 0.22f, 0.82f, 0.10f},
    };

    for (int stage = 1; stage <= 10; stage++) {
        std::vector<float> data;
        int segmentCount = std::min(stage + 1, 11);
        for (int i = 0; i < segmentCount; i++) {
            addFaceCrackStrip(data, crackSegments[i][0], crackSegments[i][1], crackSegments[i][2], crackSegments[i][3]);
        }

        crackVertexCounts.push_back((int) data.size() / 8);
        crackBuffers.push_back(MetalContext::get()->getDevice()->newBuffer(data.data(), data.size() * sizeof(float), MTL::ResourceStorageModeShared));
    }
}

BlockOutlineRenderer::~BlockOutlineRenderer() {
    if (vertexBuffer) {
        vertexBuffer->release();
    }
    for (MTL::Buffer* buffer : crackBuffers) {
        if (buffer) {
            buffer->release();
        }
    }
}

void BlockOutlineRenderer::draw(Shader& shader, const glm::vec3& blockPosition) {
    MTL::RenderCommandEncoder* encoder = shader.encoder;
    if (!encoder || !vertexBuffer) return;

    glm::mat4 model = glm::translate(glm::mat4(1.0f), blockPosition);
    shader.setMatrix4("M", model);

    encoder->setVertexBytes(&shader.uniforms, sizeof(Uniforms), 1);
    encoder->setFragmentBytes(&shader.uniforms, sizeof(Uniforms), 1);
    encoder->setVertexBuffer(vertexBuffer, 0, 0);
    encoder->drawPrimitives(MTL::PrimitiveTypeLine, NS::UInteger(0), NS::UInteger(24));
}

void BlockOutlineRenderer::drawCracks(Shader& shader, const glm::vec3& blockPosition, float progress) {
    MTL::RenderCommandEncoder* encoder = shader.encoder;
    if (!encoder || crackBuffers.empty() || progress <= 0.0f) return;

    int stage = std::min((int) (progress * (float) crackBuffers.size()), (int) crackBuffers.size() - 1);

    glm::mat4 model = glm::translate(glm::mat4(1.0f), blockPosition);
    shader.setMatrix4("M", model);

    encoder->setVertexBytes(&shader.uniforms, sizeof(Uniforms), 1);
    encoder->setFragmentBytes(&shader.uniforms, sizeof(Uniforms), 1);
    encoder->setVertexBuffer(crackBuffers[stage], 0, 0);
    encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(crackVertexCounts[stage]));
}
