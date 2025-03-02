#version 430
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_nonuniform_qualifier : require
#pragma shader_stage fragment
layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 fcol;

layout(std430, buffer_reference, buffer_reference_align = 8) readonly buffer color_t
{
	vec4 value;
};

layout(std430, set = 0, binding = 0) readonly buffer MetaBuffer
{
	color_t color;
};

layout(binding = 1) uniform sampler2D tz_textures[];
#define sample(id, uv) texture(tz_textures[id], uv)
#define texsize(id) textureSize(tz_textures[id], 0)

void main()
{
	fcol = color.value * sample(0, uv);
}
