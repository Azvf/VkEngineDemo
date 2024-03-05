#version 450

#include "include/sampling.h"
#include "include/glsl_utils.h"

layout(location = 0) in vec3 local_position;
layout(location = 0) out vec4 frag_color;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 projection;
    float roughness;
} ubo;

layout(set = 0, binding = 1) uniform samplerCube skybox_map;

void main()
{       
    vec3 N = normalize(local_position);    
    vec3 R = N;
    vec3 V = R;

    const uint SAMPLE_COUNT = 1024u;
    float totalWeight = 0.0;   
    vec3 prefilteredColor = vec3(0.0);     
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H  = ImportanceSampleGGX(Xi, N, ubo.roughness);
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if(NdotL > 0.0)
        {
            // vec4 skybox_color = GammaCorrection(texture(skybox_map, L), 2.2);
            vec4 skybox_color = texture(skybox_map, L);
            prefilteredColor += skybox_color.rgb * NdotL;
            totalWeight      += NdotL;
        }
    }
    prefilteredColor = prefilteredColor / totalWeight;

    frag_color = vec4(prefilteredColor, 1.0);
}  