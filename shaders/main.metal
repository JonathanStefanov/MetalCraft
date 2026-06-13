#include <metal_stdlib>
using namespace metal;

struct Uniforms {
    float4x4 M;
    float4x4 V;
    float4x4 P;
    float4x4 lightSpaceMatrix;
    float3 u_view_pos;
    float opacity;
    int isWater;
};

struct LightUniforms {
    float3 light_pos;
    float ambient_strength;
    float diffuse_strength;
    float specular_strength;
    float constant_att;
    float linear;
    float quadratic;
    float shininess;
};

struct VertexIn {
    float3 position [[attribute(0)]];
    float2 tex_coord [[attribute(1)]];
    float3 normal [[attribute(2)]];
};

struct VertexOut {
    float4 position [[position]];
    float4 v_col;
    float3 v_frag_coord;
    float3 v_normal;
    float2 v_t;
    float4 fragPosLightSpace;
};

// --- Main Shader ---
vertex VertexOut vertexMain(VertexIn in [[stage_in]],
                            constant Uniforms &uniforms [[buffer(1)]]) 
{
    VertexOut out;
    
    float4 frag_coord = uniforms.M * float4(in.position, 1.0);
    out.v_col = float4(in.normal * 0.5 + 0.5, 1.0);
    
    // In Metal, transpose is transpose(). We need to convert float4 to float3.
    // The original GLSL did transpose(M) * float4(normal, 0.0)
    float4x4 M_t = transpose(uniforms.M);
    out.v_normal = (M_t * float4(in.normal, 0.0)).xyz;
    out.v_frag_coord = frag_coord.xyz;
    
    out.fragPosLightSpace = uniforms.lightSpaceMatrix * float4(frag_coord.xyz, 1.0);
    out.v_t = in.tex_coord;
    
    out.position = uniforms.P * uniforms.V * frag_coord;
    
    return out;
}

float calculateShadowIntensity(float4 shadowPos, depth2d<float> shadowMap, sampler shadowSampler) {
    float3 shadowCoord = (shadowPos.xyz / shadowPos.w) * 0.5 + 0.5;
    // Metal depth is 0 to 1
    float currentDepth = shadowCoord.z;
    
    float bias = 0.001;
    float shadow = 0.0;
    
    // PCF shadow filtering
    int2 texSize = int2(shadowMap.get_width(), shadowMap.get_height());
    float2 texelSize = 1.0 / float2(texSize);
    
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float2 sampleCoord = shadowCoord.xy + float2(x, y) * texelSize;
            float pcfDepth = shadowMap.sample(shadowSampler, sampleCoord);
            shadow += (currentDepth - bias > pcfDepth) ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    return 1.0 - shadow / 2.0;
}

fragment float4 fragmentMain(VertexOut in [[stage_in]],
                             constant Uniforms &uniforms [[buffer(1)]],
                             constant LightUniforms &light [[buffer(2)]],
                             texture2d<float> tex [[texture(0)]],
                             depth2d<float> shadowMap [[texture(1)]],
                             texturecube<float> cubemap [[texture(2)]],
                             sampler texSampler [[sampler(0)]],
                             sampler shadowSampler [[sampler(1)]],
                             sampler cubeSampler [[sampler(2)]])
{
    float3 N = normalize(in.v_normal);
    float3 L = normalize(light.light_pos - in.v_frag_coord);
    float3 V = normalize(uniforms.u_view_pos - in.v_frag_coord);
    
    float3 R = reflect(-L, N);
    float cosTheta = dot(R, V);
    float spec = pow(max(cosTheta, 0.0), light.shininess);
    float specular = light.specular_strength * spec;
    
    float diffuse = light.diffuse_strength * max(dot(N, L), 0.0);
    float dist = length(light.light_pos - in.v_frag_coord);
    float attenuation = 1.0 / (light.constant_att + light.linear * dist + light.quadratic * (dist * dist));
    float combined_light = light.ambient_strength + diffuse * attenuation + specular * attenuation;
    
    float4 texColor = tex.sample(texSampler, in.v_t);
    float4 FragColor = texColor * float4(float3(combined_light), 1.0);
    
    if (uniforms.isWater != 0) {
        float3 u_view_pos_without_y = float3(uniforms.u_view_pos.x, 0.0, uniforms.u_view_pos.z);
        float3 V2 = normalize(float3(0.0, 20.0, 0.0) + u_view_pos_without_y - in.v_frag_coord);
        float3 reflection = reflect(-V2, N);
        
        // Reverse Z for reflection in Metal
        reflection.z = -reflection.z;
        
        float4 envColor = cubemap.sample(cubeSampler, reflection);
        FragColor = FragColor * 0.5 + 0.5 * float4(envColor.rgb, 1.0);
    } else {
        float shadow = calculateShadowIntensity(in.fragPosLightSpace, shadowMap, shadowSampler);
        FragColor = FragColor * shadow;
    }
    
    FragColor.a = uniforms.opacity;
    return FragColor;
}

// --- Shadow Shader ---
struct ShadowVertexOut {
    float4 position [[position]];
};

vertex ShadowVertexOut shadowVertex(VertexIn in [[stage_in]],
                                    constant Uniforms &uniforms [[buffer(1)]]) 
{
    ShadowVertexOut out;
    float4 frag_coord = uniforms.M * float4(in.position, 1.0);
    out.position = uniforms.P * uniforms.V * frag_coord;
    return out;
}

fragment float4 shadowFragment(ShadowVertexOut in [[stage_in]]) {
    // Empty fragment shader, depth is automatically written
    return float4(0);
}

// --- Cubemap Shader ---
struct CubemapVertexIn {
    float3 position [[attribute(0)]];
};

struct CubemapVertexOut {
    float4 position [[position]];
    float3 texCoords;
};

vertex CubemapVertexOut cubemapVertex(CubemapVertexIn in [[stage_in]],
                                      constant Uniforms &uniforms [[buffer(1)]]) 
{
    CubemapVertexOut out;
    out.texCoords = in.position;
    
    // Remove translation from View matrix
    float4x4 V_no_rot = uniforms.V;
    V_no_rot[3] = float4(0, 0, 0, 1);
    
    float4 pos = uniforms.P * V_no_rot * float4(in.position, 1.0);
    out.position = pos.xyww; // Force z to w so depth is 1.0
    return out;
}

fragment float4 cubemapFragment(CubemapVertexOut in [[stage_in]],
                                texturecube<float> cubemap [[texture(0)]],
                                sampler cubeSampler [[sampler(0)]]) 
{
    // Fix coordinates for metal coordinate system
    float3 coords = float3(in.texCoords.x, in.texCoords.y, -in.texCoords.z);
    return cubemap.sample(cubeSampler, coords);
}

// --- UI Shader ---
struct UIVertexIn {
    float2 position [[attribute(0)]];
    float2 tex_coord [[attribute(1)]];
};

struct UIVertexOut {
    float4 position [[position]];
    float2 v_t;
};

struct UIUniforms {
    float4x4 projection;
    float4 color;
    int type;
};

vertex UIVertexOut uiVertex(UIVertexIn in [[stage_in]],
                            constant UIUniforms &uniforms [[buffer(1)]]) 
{
    UIVertexOut out;
    out.position = uniforms.projection * float4(in.position, 0.0, 1.0);
    out.v_t = in.tex_coord;
    return out;
}

fragment float4 uiFragment(UIVertexOut in [[stage_in]],
                           constant UIUniforms &uniforms [[buffer(1)]],
                           texture2d<float> tex [[texture(0)]])
{
    constexpr sampler texSampler(mag_filter::linear, min_filter::linear, address::clamp_to_edge);
    if (uniforms.type == 0) {
        return uniforms.color;
    } else if (uniforms.type == 1) {
        float alpha = tex.sample(texSampler, in.v_t).r;
        return float4(uniforms.color.rgb, uniforms.color.a * alpha);
    } else {
        float4 tColor = tex.sample(texSampler, in.v_t);
        return tColor * uniforms.color;
    }
}

struct BlurUniforms {
    float2 resolution;
};

fragment float4 blurFragment(UIVertexOut in [[stage_in]],
                             texture2d<float> tex [[texture(0)]],
                             constant BlurUniforms &blurUni [[buffer(2)]])
{
    constexpr sampler texSampler(mag_filter::linear, min_filter::linear, address::clamp_to_edge);
    float2 tex_offset = 1.0 / blurUni.resolution;
    float4 result = float4(0.0);
    float totalWeight = 0.0;
    
    for(int x = -4; x <= 4; ++x) {
        for(int y = -4; y <= 4; ++y) {
            float2 offset = float2(x, y) * tex_offset;
            result += tex.sample(texSampler, in.v_t + offset);
            totalWeight += 1.0;
        }
    }
    return result / totalWeight;
}
