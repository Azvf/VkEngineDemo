#ifndef STRUCTURES_H
#define STRUCTURES_H

struct CameraUniformBuffer
{
    mat4 view;
    mat4 projection;
    vec4 position;

    vec4 position_padding_0;
    vec4 position_padding_1;
    vec4 position_padding_2;
};

struct ConfigUniformBuffer
{
    int enable_anti_aliasing;
    int padding;
    int rotating;
    int use_gamma_correction;
    int tone_mapping;
    int display_texture;
    
    vec2 padding_0;
    
    int placehodler_setting_0;
    int placehodler_setting_1;
    int placehodler_setting_2;
    int placehodler_setting_3;
    
    vec4 padding_1;
};

#define MAX_POINT_LIGHT_COUNT 8
struct PointLight
{
    vec3 position;
    float radius;
    vec3 color;
    float intensity;
};

struct Lights
{
    PointLight point_lights[MAX_POINT_LIGHT_COUNT];
    int    point_light_num;
   
    vec3  padding_0;
    vec4  padding_1;
    vec4  padding_2;
    vec4  padding_3;
};

#endif