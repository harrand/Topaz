#version 430
#extension GL_EXT_buffer_reference : require
#pragma shader_stage fragment
layout(location = 0) out vec4 fcol;

layout(std430, buffer_reference, buffer_reference_align = 8) readonly buffer color_t
{
	vec4 value;
};

layout(std430, set = 0, binding = 0) readonly buffer MetaBuffer
{
	color_t color;
};

void main()
{
	fcol = color.value;
	//fcol = vec4(1.0, 0.0, 1.0, 1.0);
}
