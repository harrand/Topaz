#version 430
layout(vertices = 3) out;

in vec3 vs_position_modelspace[];
in vec2 vs_texcoord_modelspace[];
in vec3 vs_normal_modelspace[];
in vec3 vs_tangent_modelspace[];

in mat4 vs_modelMatrix[];
in mat4 vs_viewMatrix[];

out vec3 tcs_position_modelspace[];
out vec2 tcs_texcoord_modelspace[];
out vec3 tcs_normal_modelspace[];
out vec3 tcs_tangent_modelspace[];

out mat4 tcs_modelMatrix[];
out mat4 tcs_viewMatrix[];

uniform float tessLevelInner = 2;
uniform float tessLevelOuter = 2;

void main()
{
	if(gl_InvocationID == 0)
	{
		gl_TessLevelInner[0] = tessLevelInner;
		gl_TessLevelOuter[0] = tessLevelOuter;
		gl_TessLevelOuter[1] = tessLevelOuter;
		gl_TessLevelOuter[2] = tessLevelOuter;
	}
	tcs_position_modelspace[gl_InvocationID] = vs_position_modelspace[gl_InvocationID];
	tcs_texcoord_modelspace[gl_InvocationID] = vs_texcoord_modelspace[gl_InvocationID];
	tcs_normal_modelspace[gl_InvocationID] = vs_normal_modelspace[gl_InvocationID];
	tcs_tangent_modelspace[gl_InvocationID] = vs_tangent_modelspace[gl_InvocationID];
	tcs_modelMatrix[gl_InvocationID] = vs_modelMatrix[gl_InvocationID];
	tcs_viewMatrix[gl_InvocationID] = vs_viewMatrix[gl_InvocationID];
}