// Vertex Shader version 4.30
#version 430

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;

out vec2 texcoord_modelspace;

uniform mat4 m;
uniform mat4 v;
uniform mat4 p;

void share()
{
	texcoord_modelspace = texcoord;
}

void main()
{
	share();
	gl_Position = (p * v * m) * vec4(position, 1.0);
}