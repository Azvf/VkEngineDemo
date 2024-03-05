#version 450

#include "include/brdf.h"
#include "include/glsl_utils.h"
#include "include/tone_mapping.h"
#include "include/structures.h"

layout(set = 0, binding = 0) uniform UniformBufferObject {
    ConfigUniformBuffer config;
    CameraUniformBuffer camera;
    Lights lights;
} ubo;

layout(set = 0, binding = 1) uniform sampler2D base_color_sampler;
layout(set = 0, binding = 2) uniform sampler2D normal_sampler;
layout(set = 0, binding = 3) uniform sampler2D ao_sampler;
layout(set = 0, binding = 4) uniform sampler2D metallic_sampler;
layout(set = 0, binding = 5) uniform sampler2D roughness_sampler;

layout(set = 0, binding = 6) uniform samplerCube skybox_irradiance_sampler;
layout(set = 0, binding = 7) uniform samplerCube skybox_prefilter_sampler;
layout(set = 0, binding = 8) uniform sampler2D brdf_lut_sampler;

layout(location = 0) in vec2 in_uv;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_tangent;
layout(location = 3) in vec3 in_world_position;

layout(location = 0) out vec4 frag_color;

vec3 CalculateWorldNormal() {
    vec3 tangent_normal = texture(normal_sampler, in_uv).xyz * 2.0 - 1.0;

    vec3 N = normalize(in_normal);
    vec3 T = normalize(in_tangent.xyz);
    vec3 B = normalize(cross(N, T));

    mat3 TBN = mat3(T, B, N);
    return normalize(TBN * tangent_normal);
}

float SmoothDistanceAtt(float sqr_distance, float inv_sqr_attenuation_radius)
{
	float factor = sqr_distance * inv_sqr_attenuation_radius;
	float smooth_factor = clamp(1.0 - factor * factor, 0.0, 1.0);
	return smooth_factor * smooth_factor;
}

float GetDistanceAttenuation(vec3 unnormalized_light_vector, float inv_sqr_attenuation_radius)
{
	float sqr_distance = dot(unnormalized_light_vector, unnormalized_light_vector);
	float attenuation = 1.0 / (max(sqr_distance, inv_sqr_attenuation_radius));
	attenuation *= SmoothDistanceAtt(sqr_distance, inv_sqr_attenuation_radius);
	
	return attenuation;
}

vec3 PiccoloRadiance(PointLight light, vec3 position, vec3 N, vec3 L) {
    // point light
    float distance             = length(light.position - position);
    float distance_attenuation = 1.0 / (distance * distance + 1.0);
    float radius_attenuation   = 1.0 - ((distance * distance) / (light.radius * light.radius));
    float light_attenuation = radius_attenuation * distance_attenuation * max(dot(N, L), 0.0);
    
    vec3 radiance = light.color * light.intensity * light_attenuation;
    
    return radiance;
}

void main() 
{
    if (ubo.config.display_texture == 1) {
        frag_color = vec4(texture(base_color_sampler, in_uv).rgb, 1.0);
        return;
    } else if (ubo.config.display_texture == 2) {
        frag_color = vec4(texture(normal_sampler, in_uv).rgb, 1.0);
        return;
    } else if (ubo.config.display_texture == 3) {
        frag_color = vec4(texture(ao_sampler, in_uv).rgb, 1.0);
        return;
    } else if (ubo.config.display_texture == 4) {
        frag_color = vec4(texture(metallic_sampler, in_uv).rgb, 1.0);
        return;
    } else if (ubo.config.display_texture == 5) {
        frag_color = vec4(texture(roughness_sampler, in_uv).rgb, 1.0);
        return;
    } else if (ubo.config.display_texture == 6) {
        frag_color = vec4(in_uv, 1.0, 1.0);
        return;
    }
    
    vec3 base_color = texture(base_color_sampler, in_uv).rgb;
    float metallic = texture(metallic_sampler, in_uv).r;
    float roughness = texture(roughness_sampler, in_uv).r;
    float ao = texture(ao_sampler, in_uv).r;

    vec3 Lo = vec3(0.0);
    vec3 Libl = vec3(0.0);
    
    vec3 N = CalculateWorldNormal();
    vec3 V = normalize(ubo.camera.position.xyz - in_world_position);
    vec3 R = reflect(-V, N); 
    
    float NdotV = dot(N, V);

    // point light
    for (int i = 0; i < ubo.lights.point_light_num; i++) {
        vec3 light_position = ubo.lights.point_lights[i].position;
        vec3 light_color = ubo.lights.point_lights[i].color;
        float light_radius = ubo.lights.point_lights[i].radius;
        float light_intensity = ubo.lights.point_lights[i].intensity;
        
        float inv_sqr_radius = 1 / (light_radius * light_radius);
        vec3 unnormalized_light_vector = light_position - in_world_position;

        vec3 L = normalize(light_position - in_world_position);

#if 0
        vec3 radiance = 
            light_color * light_intensity
            * GetDistanceAttenuation(unnormalized_light_vector, inv_sqr_radius) 
            * max(dot(N, L), 0.0);
#else
        vec3 radiance = PiccoloRadiance(ubo.lights.point_lights[i], in_world_position, N, L);
#endif

        // Lo += BRDF(N, V, L, base_color, metallic, roughness, ao) * light_intensity * max(dot(N, L), 0.0);
        Lo += BRDF(N, V, L, base_color, metallic, roughness, ao) * radiance;
    }
    
    // // image based lighting
    // {
    //     vec3 irradiance = texture(skybox_irradiance_sampler, N).rgb;
    //     vec3 fd    = irradiance * base_color;

    //     vec3 F0 = GetF0(base_color, metallic);
    //     vec3 F = FrR(NdotV, F0, roughness);
    //     vec2 brdf_lut = texture(brdf_lut_sampler, vec2(max(NdotV, 0.0), roughness)).rg;

    //     float lod        = roughness * MAX_LOD_LEVEL;
    //     vec3  prefiltered_color = textureLod(skybox_prefilter_sampler, R, lod).rgb;
    //     vec3  fs   = prefiltered_color * (F * brdf_lut.x + brdf_lut.y);
        
    //     vec3 kS = F;
    //     vec3 kD = 1.0 - kS;
    //     kD *= 1.0 - metallic;
    //     Libl = (kD * fd + fs);
    // }

    frag_color = vec4(Lo + Libl, 1.0);

    // tone mapping
    if (bool(ubo.config.tone_mapping)) {
        frag_color = vec4(ACES(frag_color.rgb), 1.0);
    }

    // gamma correction
    if (bool(ubo.config.use_gamma_correction)) {
        frag_color = GammaCorrection(frag_color, 2.2);
    }
}
