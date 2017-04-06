#version 430
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 vs_position_modelspace[3];
in vec2 vs_texcoord_modelspace[3];
in vec3 vs_normal_modelspace[3];

in mat4 vs_modelMatrix[3];
in mat4 vs_viewMatrix[3];
in mat3 vs_tbnMatrix[3];

out vec3 position_modelspace;
out vec2 texcoord_modelspace;
out vec3 normal_modelspace;

out mat4 modelMatrix;
out mat4 viewMatrix;
out mat3 tbnMatrix;

void main()
{
	for(unsigned int i = 0; i < gl_in.length(); i++)
	{
		position_modelspace = vs_position_modelspace[i];
		texcoord_modelspace = vs_texcoord_modelspace[i];
		normal_modelspace = vs_normal_modelspace[i];
		modelMatrix = vs_modelMatrix[i];
		viewMatrix = vs_viewMatrix[i];
		tbnMatrix = vs_tbnMatrix[i];
		gl_Position = gl_in[i].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}