// Vertex Shader version 4.30
#version 430

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;

out vec3 position_modelspace;
out vec2 texcoord_modelspace;
out vec3 normal_modelspace;

out mat4 modelMatrix;
out mat4 viewMatrix;
out mat3 tbnMatrix;

uniform mat4 m;
uniform mat4 v;
uniform mat4 p;

void share()
{
	position_modelspace = position;
	texcoord_modelspace = texcoord;
	normal_modelspace = normal;
	
	modelMatrix = m;
	viewMatrix = v;
	
	vec3 normal_worldspace = normalize((m * vec4(normal, 0.0)).xyz);
	vec3 tangent_worldspace = normalize((m * vec4(tangent, 0.0)).xyz);
	
	// Gramm-Schmidt Process
	tangent_worldspace = normalize(tangent_worldspace - dot(tangent_worldspace, normal_worldspace) * normal_worldspace);
	
	vec3 bitangent_worldspace = cross(tangent_worldspace, normal_worldspace);
	
	tbnMatrix = transpose(mat3(tangent_worldspace, bitangent_worldspace, normal_worldspace));
}

void main()
{
	share();
	gl_Position = (p * v * m) * vec4(position, 1.0);
}