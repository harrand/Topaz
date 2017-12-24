#version 430

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;

uniform mat4 m;
uniform mat4 v;
uniform mat4 p;

out vec2 texture_coordinate;

void main()
{
	texture_coordinate =  texcoord;
	gl_Position = p * v * m * vec4(position, 1.0);
}