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

struct object_t
{
	vec3 world_position;
};

layout(scalar, buffer_reference, buffer_reference_align = 8) readonly buffer vertex_data_t
{
	vertex_t data[];
};

layout(scalar, buffer_reference, buffer_reference_align = 8) readonly buffer object_data_t
{
	object_t data[];
};

layout(std430, set = 0, binding = 0) readonly buffer MetaBuffer
{
	vertex_data_t vertex;
	object_data_t object;
};

layout(location = 0) out vec2 uv;
void main()
{
	vertex_t cur_vertex = vertex.data[gl_VertexIndex];
	uint data_id = gl_BaseInstance;
	if(data_id == -1)
	{
		// no object data associated.
	}
	else
	{
		object_t cur_object = object.data[data_id];
		cur_vertex.pos += cur_object.world_position;
	}
	uv = cur_vertex.texcoord;

	gl_Position = vec4(cur_vertex.pos, 1.0);
}
