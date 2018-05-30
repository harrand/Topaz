// Vertex Shader version 4.30
#version 430

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;

out vec3 position_modelspace;
out vec2 texcoord_modelspace;

out mat4 model_matrix;
out mat4 view_matrix;
out mat4 projection_matrix;

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

	model_matrix = m;
	view_matrix = v;
	projection_matrix = p;
}

void main()
{
	share();
	gl_Position = (p * v * m) * vec4(position_modelspace, 1.0);
}