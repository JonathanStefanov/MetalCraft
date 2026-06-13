#pragma once

#include <simd/simd.h>

struct Uniforms {
    simd::float4x4 M;
    simd::float4x4 V;
    simd::float4x4 P;
    simd::float4x4 lightSpaceMatrix;
    simd::float3 u_view_pos;
    float opacity;
    int isWater;
};

struct LightUniforms {
    simd::float3 light_pos;
    float ambient_strength;
    float diffuse_strength;
    float specular_strength;
    float constant_att;
    float linear;
    float quadratic;
    float shininess;
};
