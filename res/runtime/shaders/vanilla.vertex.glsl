// Vertex Shader version 4.30
#version 430

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;

out vec3 positionShared;
out vec2 texcoordShared;
out vec3 normalShared;
out mat4 viewShared;
out vec3 eyeDirectionCamSpace;

uniform mat4 m;
uniform mat4 v;
uniform mat4 p;

void main()
{
	gl_Position = (p * v * m) * vec4(position, 1.0);
	positionShared = (m * vec4(position, 1.0)).xyz;
	normalShared = normalize((m * vec4(normal, 0.0)).xyz);
	texcoordShared = texcoord;
	viewShared = v;
	vec3 vertexPos_camSpace = (v * m * vec4(position, 1.0)).xyz;
	eyeDirectionCamSpace = vec3(0,0,0) - vertexPos_camSpace;
}