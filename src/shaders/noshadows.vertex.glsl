// Vertex Shader version 4.30
#version 430

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;
layout(location = 5) in vec3 positions_instance;
layout(location = 6) in vec3 rotations_instance;
layout(location = 7) in vec3 scales_instance;

out vec3 vs_position_modelspace;
out vec2 vs_texcoord_modelspace;
out vec3 vs_normal_modelspace;

out mat4 vs_model_matrix;
out mat4 vs_view_matrix;
out mat4 vs_projection_matrix;
out mat3 vs_tbn_matrix;

uniform vec3 position_uniform;
uniform vec3 rotation_uniform;
uniform vec3 scale_uniform;
uniform mat4 m;
uniform mat4 v;
uniform mat4 p;
uniform float displacement_factor;
uniform bool is_instanced;

uniform sampler2D displacement_map_sampler;

void share()
{
	vs_position_modelspace = position;
	vs_texcoord_modelspace = texcoord;
	vs_normal_modelspace = normal;
	vs_position_modelspace += normal * texture2D(displacement_map_sampler, vs_texcoord_modelspace).r * displacement_factor;
	
	vs_model_matrix = m;
	vs_view_matrix = v;
	vs_projection_matrix = p;
	
	vec3 normal_cameraspace = normalize((v * m * vec4(vs_normal_modelspace, 0.0)).xyz);
	vec3 tangent_cameraspace = normalize((m * vec4(tangent, 0.0)).xyz);
	
	// Gramm-Schmidt Process
	tangent_cameraspace = normalize(tangent_cameraspace - dot(tangent_cameraspace, normal_cameraspace) * normal_cameraspace);
	
	vec3 bitangent_cameraspace = cross(tangent_cameraspace, normal_cameraspace);
	
	vs_tbn_matrix = transpose(mat3(tangent_cameraspace, bitangent_cameraspace, normal_cameraspace));
}

mat4 translate(vec3 position)
{
	return mat4(vec4(1, 0, 0, position.x), vec4(0, 1, 0, position.y), vec4(0, 0, 1, position.z), vec4(0, 0, 0, 1));
}

mat4 rotate_x(float angle)
{
	return mat4(vec4(1, 0, 0, 0), vec4(0, cos(angle), -sin(angle), 0), vec4(0, sin(angle), cos(angle), 0), vec4(0, 0, 0, 1));
}

mat4 rotate_y(float angle)
{
	return mat4(vec4(cos(angle), 0, sin(angle), 0), vec4(0, 1, 0, 0), vec4(-sin(angle), 0, cos(angle), 0), vec4(0, 0, 0, 1));
}

mat4 rotate_z(float angle)
{
	return mat4(vec4(cos(angle), -sin(angle), 0, 0), vec4(sin(angle), cos(angle), 0, 0), vec4(0, 0, 1, 0), vec4(0, 0, 0, 1));
}

mat4 rotate(vec3 euler_rotation)
{
	return rotate_z(euler_rotation.z) * rotate_y(euler_rotation.y) * rotate_x(euler_rotation.x);
}

mat4 scale(vec3 scale)
{
	return mat4(vec4(scale.x, 0, 0, 0), vec4(0, scale.y, 0, 0), vec4(0, 0, scale.z, 0), vec4(0, 0, 0, 1));
}

void main()
{
	share();
	/*
	vec4 rx = rotations_instance_x;
	vec4 ry = rotations_instance_y;
	vec4 rz = rotations_instance_z;
	vec4 rw = rotations_instance_w;
	*/
	mat4 model = scale(scale_uniform + scales_instance) * rotate(rotation_uniform + rotations_instance) * translate(position_uniform);
	//mat4 rotation_instance = mat4(rotations_instance_x.wzyx, rotations_instance_y.wzyx, rotations_instance_z.wzyx, rotations_instance_w.wzyx);
	if(is_instanced)
	{
		vs_model_matrix = model;
		gl_Position = p * v * ((transpose(model) * vec4(vs_position_modelspace, 1.0)) + vec4(positions_instance, 0));
	}
	else
		gl_Position = (p * v * m) * vec4(vs_position_modelspace, 1.0);
}