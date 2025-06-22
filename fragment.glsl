#version 430
#extension GL_EXT_nonuniform_qualifier : require
#pragma shader_stage fragment
layout(location = 0) out vec4 fcol;

void main()
{
	fcol = vec4(1.0, 0.0, 0.7, 1.0);
}
