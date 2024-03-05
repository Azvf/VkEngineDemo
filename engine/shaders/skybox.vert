#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 inv_model_view_proj;
	int show_skybox_index;
	float skybox_prefilter_mip_level;
} ubo;

layout(location = 0) out vec4 out_ray;

void main() 
{
	float x = -1.0 + float((gl_VertexIndex & 1) << 2);
	float y = -1.0 + float((gl_VertexIndex & 2) << 1);
    gl_Position = vec4(x, y, 1.0, 1.0);
	out_ray = ubo.inv_model_view_proj * vec4(gl_Position.xy, 1.0,  1.0);
}