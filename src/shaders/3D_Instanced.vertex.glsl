// Vertex Shader version 4.30
#version 430

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 4) in vec4 instancing_model_x;
layout(location = 5) in vec4 instancing_model_y;
layout(location = 6) in vec4 instancing_model_z;
layout(location = 7) in vec4 instancing_model_w;

out vec2 texcoord_modelspace;

uniform mat4 m;
uniform mat4 v;
uniform mat4 p;

mat4 model_matrix;

uniform bool is_instanced = false;

void share()
{
	texcoord_modelspace = texcoord;
	mat4 model_instanced = transpose(mat4(instancing_model_x, instancing_model_y, instancing_model_z, instancing_model_w));

	if(is_instanced)
		model_matrix = model_instanced;
	else
		model_matrix = m;
}

void main()
{
	share();
	gl_Position = (p * v * model_matrix) * vec4(position, 1.0);
}