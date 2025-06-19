#version 430
#extension GL_EXT_buffer_reference : require
#pragma shader_stage vertex
vec2 positions[3] = vec2[](vec2(0.0, 0.5), vec2(0.5, -0.5), vec2(-0.5, -0.5));
vec2 texcoords[3] = vec2[](vec2(0.5, 1.0), vec2(1.0, 0.0), vec2(0.0, 0.0));

void main()
{
	gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}
