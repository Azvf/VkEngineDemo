#ifndef BRDF_H
#define BRDF_H

#include "constants.h"

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float a = roughness;
    float k = (a * a) / 2.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}  

float SchlickGGX_D(vec3 norm, vec3 h, float roughness){
	float NoH = dot(norm, h);
	if(NoH < 0)
		return 0;
	
	float alpha = roughness * roughness;
	
	float alpha2 = alpha * alpha;
	float cos2Theta = NoH * NoH;
	
	float t = (alpha2 - 1) * cos2Theta + 1;
	
	return alpha2 / (PI * t * t);
}

float SchlickGGX_G1_IBL(vec3 norm, vec3 w, float roughness) {
	float k = sqrt(roughness) / 2.0;
	
	float NoW = max(0.f, dot(norm, w));
	return NoW / (NoW * (1 - k) + k);
}

float SchlickGGX_G_IBL(vec3 norm, vec3 wo, vec3 wi, float roughness) {
	return SchlickGGX_G1_IBL(norm, wo, roughness) * SchlickGGX_G1_IBL(norm, wi, roughness);
}

float SchlickGGX_G1(vec3 norm, vec3 w, float roughness) {
	float k = (roughness+1) * (roughness+1) / 8;
	
	float NoW = max(0.f, dot(norm, w));
	return NoW / (NoW * (1 - k) + k);
}

float SchlickGGX_G(vec3 norm, vec3 wo, vec3 wi, float roughness) {
	return SchlickGGX_G1(norm, wo, roughness) * SchlickGGX_G1(norm, wi, roughness);
}

vec3 GetF0(vec3 albedo, float metallic) {
    return mix(vec3(0.04), albedo, metallic);
}

// Unreal Engine fast version
vec3 Fr(float costheta, vec3 albedo, float metallic) {
	vec3 F0 = GetF0(albedo, metallic);
	float HoW = max(costheta, 0.0);
	return F0 + exp2((-5.55473f * HoW - 6.98316f) * HoW) * (vec3(1.0f) - F0);
}

vec3 Fr(vec3 w, vec3 h, vec3 albedo, float metallic) {
	vec3 F0 = GetF0(albedo, metallic);
	float HoW = max(dot(h, w), 0.0);
	return F0 + exp2((-5.55473f * HoW - 6.98316f) * HoW) * (vec3(1.0f) - F0);
}

// traditional Schlick version
vec3 FrR(float costheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - costheta, 5.0);
}

vec3 FrR(vec3 wo, vec3 norm, vec3 F0, float roughness) {
	float cosTheta = max(dot(wo, norm), 0);
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 BRDF(vec3 norm, vec3 wo, vec3 wi, vec3 albedo, float metallic, float roughness, float ao) {
	vec3 wh = normalize(wo + wi);
	
	float D = SchlickGGX_D(norm, wh, roughness);
	float G = SchlickGGX_G(norm, wo, wi, roughness);
	vec3 F = Fr(wo, wh, albedo, metallic);
	
	vec3 specular = D * G * F / (4.0f * dot(wh, wo) * dot(wh, wi));
	
	vec3 diffuse = albedo * INV_PI;
	
	vec3 kS = 1 - F;
	vec3 kD = (1-metallic) * (1 - kS);
	
	vec3 rst = kD * diffuse + specular;

	//return ao * rst;
	return rst;
}

#endif
