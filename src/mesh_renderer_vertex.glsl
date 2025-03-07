#version 460 core
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require
#pragma shader_stage vertex
vec2 positions[3] = vec2[](vec2(0.0, 0.5), vec2(0.5, -0.5), vec2(-0.5, -0.5));
vec2 texcoords[3] = vec2[](vec2(0.5, 1.0), vec2(1.0, 0.0), vec2(0.0, 0.0));

struct vertex_t
{
	vec3 pos;
	vec2 texcoord;
};

layout(scalar, buffer_reference, buffer_reference_align = 8) readonly buffer vertex_data_t
{
	vertex_t data[];
};

layout(std430, set = 0, binding = 0) readonly buffer MetaBuffer
{
	vertex_data_t vertex;
};

layout(location = 0) out vec2 uv;
void main()
{
	vertex_t cur_vertex = vertex.data[gl_VertexIndex];
	gl_Position = vec4(cur_vertex.pos, 1.0);
	int data_id = gl_BaseInstance;
	uv = cur_vertex.texcoord;
}
