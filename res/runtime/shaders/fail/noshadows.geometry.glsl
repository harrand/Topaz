#version 430
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 tes_position_modelspace[];
in vec2 tes_texcoord_modelspace[];
in vec3 tes_normal_modelspace[];

in vec3 tes_patch_distance[];

in mat4 tes_modelMatrix[];
in mat4 tes_viewMatrix[];
in mat3 tes_tbnMatrix[];

out vec3 gs_position_modelspace;
out vec2 gs_texcoord_modelspace;
out vec3 gs_normal_modelspace;

out mat4 gs_modelMatrix;
out mat4 gs_viewMatrix;
out mat3 gs_tbnMatrix;

void main()
{
	vec3 A = tes_position_modelspace[2] - tes_position_modelspace[0];
	vec3 B = tes_position_modelspace[1] - tes_position_modelspace[0];
	
	for(unsigned int i = 0; i < gl_in.length(); i++)
	{	
		gs_position_modelspace = tes_position_modelspace[i];
		gs_texcoord_modelspace = tes_texcoord_modelspace[i];
		gs_normal_modelspace = tes_normal_modelspace[i];
		gs_modelMatrix = tes_modelMatrix[i];
		gs_viewMatrix = tes_viewMatrix[i];
		gs_tbnMatrix = tes_tbnMatrix[i];
		gl_Position = gl_in[i].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
	
}