// Vertex Shader version 4.30
#version 430

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;

out vec3 position_modelspace;
out vec2 texcoord_modelspace;

out mat4 model_matrix;
out mat4 view_matrix;
out mat4 projection_matrix;
out mat3 tbn_matrix;

uniform vec3 position_uniform;
uniform vec3 rotation_uniform;
uniform vec3 scale_uniform;
uniform mat4 m;
uniform mat4 v;
uniform mat4 p;
uniform float displacement_factor;

uniform sampler2D displacement_map_sampler;

void share()
{
	position_modelspace = position;
	texcoord_modelspace = texcoord;
	position_modelspace += normal * texture2D(displacement_map_sampler, texcoord_modelspace).r * displacement_factor;

	model_matrix = m;
	view_matrix = v;
	projection_matrix = p;

	vec3 bitangent = cross(tangent, normal);

	vec3 normal_modelspace = normalize((m * vec4(normal, 0.0)).xyz);
	vec3 tangent_modelspace = normalize((m * vec4(tangent, 0.0)).xyz);
	vec3 bitangent_modelspace = normalize((m * vec4(bitangent, 0.0)).xyz);

	// Gramm-Schmidt Process
	tangent_modelspace = normalize(tangent_modelspace - dot(tangent_modelspace, normal_modelspace) * normal_modelspace);

	tbn_matrix = mat3(tangent_modelspace, bitangent_modelspace, normal_modelspace);
}

void main()
{
	share();
	gl_Position = (p * v * m) * vec4(position_modelspace, 1.0);
}