#version 430
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 vs_position_modelspace[3];
in vec2 vs_texcoord_modelspace[3];
in vec3 vs_normal_modelspace[3];

in mat4 vs_model_matrix[3];
in mat4 vs_view_matrix[3];
in mat4 vs_projection_matrix[3];
in mat3 vs_tbn_matrix[3];

out vec3 position_modelspace;
out vec3 cube_position_modelspace;
out vec2 texcoord_modelspace;
out vec3 normal_modelspace;

out mat4 model_matrix;
out mat4 view_matrix;
out mat3 tbn_matrix;

void main()
{
	for(uint i = 0u; i < gl_in.length(); i++)
	{
		position_modelspace = vs_position_modelspace[i];
		texcoord_modelspace = vs_texcoord_modelspace[i];
		normal_modelspace = vs_normal_modelspace[i];
		model_matrix = vs_model_matrix[i];
		view_matrix = vs_view_matrix[i];
		tbn_matrix = vs_tbn_matrix[i];
		gl_Position = gl_in[i].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}