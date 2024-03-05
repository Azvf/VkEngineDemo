#version 450
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 3) in vec3 in_uv;

layout (location = 0) out vec3 out_local_position;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 projection;
    float roughness;
} ubo;

void main()
{
    out_local_position = in_position;
    gl_Position =  ubo.projection * ubo.view * vec4(out_local_position, 1.0);
}