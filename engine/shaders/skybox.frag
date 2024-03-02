#version 450

layout(set = 0, binding = 1) uniform samplerCube uSkybox;

layout(early_fragment_tests) in;

layout(location = 0) in vec4 vRay;

layout(location = 0) out vec4 oColor;

vec3 ndcxyz_to_uvw(vec3 ndcxy) { return ndcxy * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5); }

void main() 
{
	oColor = vec4(textureLod(uSkybox, vRay.xyz / vRay.w, 0.0).rgb, 1.0);
	// oColor = vec4(ndcxyz_to_uvw(vRay.xyz / vRay.w), 1.0);
}
