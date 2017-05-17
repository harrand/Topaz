#version 430
layout(triangles, equal_spacing, cw) in;

in vec3 tcs_position_modelspace[];
in vec2 tcs_texcoord_modelspace[];
in vec3 tcs_normal_modelspace[];
in vec3 tcs_tangent_modelspace[];

in mat4 tcs_modelMatrix[];
in mat4 tcs_viewMatrix[];

out vec3 tes_position_modelspace;
out vec2 tes_texcoord_modelspace;
out vec3 tes_normal_modelspace;

out vec3 tes_patch_distance;

out mat4 tes_modelMatrix;
out mat4 tes_viewMatrix;
out mat3 tes_tbnMatrix;

uniform mat4 p;
//uniform mat4 v;
//uniform mat4 m;

void main()
{
	tes_texcoord_modelspace = tcs_texcoord_modelspace[0];
	tes_modelMatrix = tcs_modelMatrix[0];
	tes_viewMatrix = tcs_viewMatrix[0];
	
	vec3 p0 = gl_TessCoord.x * tcs_position_modelspace[0];
	vec3 p1 = gl_TessCoord.y * tcs_position_modelspace[1];
	vec3 p2 = gl_TessCoord.z * tcs_position_modelspace[2];
	
	vec3 n0 = gl_TessCoord.x * tcs_normal_modelspace[0];
	vec3 n1 = gl_TessCoord.y * tcs_normal_modelspace[1];
	vec3 n2 = gl_TessCoord.z * tcs_normal_modelspace[2];
	
	vec3 t0 = gl_TessCoord.x * tcs_tangent_modelspace[0];
	vec3 t1 = gl_TessCoord.x * tcs_tangent_modelspace[1];
	vec3 t2 = gl_TessCoord.x * tcs_tangent_modelspace[2];
	
	tes_patch_distance = gl_TessCoord;
	tes_position_modelspace = normalize(p0 + p1 + p2);
	tes_normal_modelspace = normalize(n0 + n1 + n2);
	tes_tbnMatrix[0] = normalize(t0 + t1 + t2);
	tes_tbnMatrix[2] = tes_normal_modelspace;
	tes_tbnMatrix[1] = normalize(cross(tes_tbnMatrix[0], tes_tbnMatrix[2]));
	gl_Position = p * tes_viewMatrix * tes_modelMatrix * vec4(tes_position_modelspace, 1);
	//gl_Position = vec4(tes_position_modelspace, 1);
}