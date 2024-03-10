#version 450

#include "include/constants.h"
#include "include/glsl_utils.h"

layout(location = 0) in vec3 local_position;
layout(location = 0) out vec4 frag_color;

layout(set = 0, binding = 1) uniform samplerCube skybox_map;

void main()
{
	vec3 N = normalize(local_position);
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, N));
	up = cross(N, right);

    float deltaPhi = (2.0f * PI) / 180.0f;
    float deltaTheta = (0.5f * PI) / 64.0f;

	vec3 color = vec3(0.0);
	uint sampleCount = 0u;
	for (float phi = 0.0; phi < TWO_PI; phi += deltaPhi) {
		for (float theta = 0.0; theta < HALF_PI; theta += deltaTheta) {
			vec3 tempVec = cos(phi) * right + sin(phi) * up;
			vec3 sampleVector = cos(theta) * N + sin(theta) * tempVec;
            vec4 skybox_color = GammaCorrection(texture(skybox_map, sampleVector), 2.2);
            // vec4 skybox_color = texture(skybox_map, sampleVector);
			color += skybox_color.rgb * cos(theta) * sin(theta);
			sampleCount++;
		}
	}
	frag_color = vec4(PI * color / float(sampleCount), 1.0);
}
