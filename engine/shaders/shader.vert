#version 450
#include "include/structures.h"

layout(set = 0, binding = 0) uniform UniformBufferObject {
    ConfigUniformBuffer config;
    CameraUniformBuffer camera;
    Lights lights;
} ubo;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 texcoord;

layout(location = 0) out vec2 out_uv;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec3 out_tangent;
layout(location = 3) out vec3 out_world_position;

void main() {
    gl_Position = ubo.camera.projection * ubo.camera.view * vec4(position, 1.0);

    out_uv = texcoord;
    out_normal = normal;
    out_tangent = tangent;
    out_world_position = position;
}
