// Vertex Shader version 4.30
#version 430

#include "transform_utility.header.glsl"

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec4 instancing_model_x;
layout(location = 5) in vec4 instancing_model_y;
layout(location = 6) in vec4 instancing_model_z;
layout(location = 7) in vec4 instancing_model_w;

out MeshData
{
    vec3 position_modelspace;
    vec4 position_lightspace;
    vec2 texcoord_modelspace;
    vec3 normal_modelspace;
} output_mesh_data;

out CameraData
{
    vec3 eye_direction_cameraspace;
    vec3 light_direction_cameraspace;
} output_camera_data;

out MatrixBlock
{
    mat4 model;
    mat4 view;
    mat4 projection;
    mat3 tbn;
} output_matrices;

uniform mat4 m;
uniform mat4 v;
uniform mat4 p;
uniform mat4 light_viewprojection;
uniform float displacement_factor = 0.0f;
uniform bool has_displacement_map = false;

uniform bool is_instanced = false;

uniform sampler2D displacement_map_sampler;

mat4 model_instanced;

void share()
{
	output_mesh_data.position_modelspace = position;
	output_mesh_data.texcoord_modelspace = texcoord;
	output_mesh_data.normal_modelspace = normal;
	if(has_displacement_map)
		output_mesh_data.position_modelspace += normal * texture2D(displacement_map_sampler, texcoord).r * displacement_factor;

	model_instanced = transpose(mat4(instancing_model_x, instancing_model_y, instancing_model_z, instancing_model_w));

	if(is_instanced)
		output_matrices.model = model_instanced;
	else
		output_matrices.model = m;
	output_matrices.view = v;
	output_matrices.projection = p;

	vec3 position_cameraspace = transform3f(output_mesh_data.position_modelspace, output_matrices.view * output_matrices.model, true);
	output_camera_data.eye_direction_cameraspace = vec3(0, 0, 0) - position_cameraspace;

	// edit this as you wish
	const vec3 light_position_cameraspace = vec3(0, 0, 0);
	output_camera_data.light_direction_cameraspace = light_position_cameraspace + output_camera_data.eye_direction_cameraspace;
	
	vec3 bitangent = cross(tangent, normal);
	
	vec3 normal_cameraspace = normalize((v * m * vec4(normal, 0.0)).xyz);
	vec3 tangent_cameraspace = normalize((v * m * vec4(tangent, 0.0)).xyz);
	vec3 bitangent_cameraspace = normalize((v * m * vec4(bitangent, 0.0)).xyz);
	
	// Gramm-Schmidt Process
	tangent_cameraspace = normalize(tangent_cameraspace - dot(tangent_cameraspace, normal_cameraspace) * normal_cameraspace);
	
	output_matrices.tbn = transpose(mat3(tangent_cameraspace, bitangent_cameraspace, normal_cameraspace));
}

void main()
{
	share();
	if(is_instanced)
	{
		gl_Position = p * v * (model_instanced * vec4(output_mesh_data.position_modelspace, 1.0));
		output_mesh_data.position_lightspace = light_viewprojection * model_instanced * vec4(output_mesh_data.position_modelspace, 1.0);
	}
	else
	{
		gl_Position = (p * v * m) * vec4(output_mesh_data.position_modelspace, 1.0);
		output_mesh_data.position_lightspace = light_viewprojection * m * vec4(output_mesh_data.position_modelspace, 1.0);
	}
}