#version 450

#include "include/constants.h"
#include "include/glsl_utils.h"

layout(location = 0) in vec3 local_position;
layout(location = 0) out vec4 frag_color;

layout(set = 0, binding = 1) uniform samplerCube skybox_map;

void main()
{		
    vec3 N = normalize(local_position);

    vec3 irradiance = vec3(0.0);   
    
    // tangent space calculation from origin point
    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up         = normalize(cross(N, right));
       
    float sampleDelta = 0.025;
    float nrSamples = 0.0f;
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

            // vec4 skybox_color = GammaCorrection(texture(skybox_map, sampleVec), 2.2);
            vec4 skybox_color = texture(skybox_map, sampleVec);
            irradiance += skybox_color.rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    
    frag_color = vec4(irradiance, 1.0);
}