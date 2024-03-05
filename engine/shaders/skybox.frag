#version 450

#include "include/glsl_utils.h"

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 inv_model_view_proj;
	int show_skybox_index;
	float skybox_prefilter_mip_level;
} ubo;

layout(set = 0, binding = 1) uniform samplerCube skybox_sampler;
layout(set = 0, binding = 2) uniform samplerCube skybox_irradiance_sampler;
layout(set = 0, binding = 3) uniform samplerCube skybox_prefilter_sampler;

layout(location = 0) in vec4 in_ray;

layout(location = 0) out vec4 frag_color;

void main() 
{
	if (ubo.show_skybox_index == 0) {
		frag_color = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	} else if (ubo.show_skybox_index == 1) {
		frag_color = vec4(textureLod(skybox_sampler, in_ray.xyz / in_ray.w, 0.0).rgb, 1.0);
		// // gamma correction
		// frag_color = GammaCorrection(frag_color, 2.2);
	}
	 else if (ubo.show_skybox_index == 2) {
		frag_color = vec4(textureLod(skybox_irradiance_sampler, in_ray.xyz / in_ray.w, 0.0).rgb, 1.0);
	} else if (ubo.show_skybox_index == 3) {
		float lod = ubo.skybox_prefilter_mip_level;
		frag_color = vec4(textureLod(skybox_prefilter_sampler, in_ray.xyz / in_ray.w, lod).rgb, 1.0);
	}
}
