// Vertex Shader version 4.30
#version 430

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;

out vec3 position_modelspace;
out vec2 texcoord_modelspace;
out vec3 normal_modelspace;
out vec3 eye_direction_cameraspace;
out vec3 light_direction_cameraspace;

out mat4 model_matrix;
out mat4 view_matrix;
out mat4 projection_matrix;
out mat3 tbn_matrix;

uniform mat4 m;
uniform mat4 v;
uniform mat4 p;
uniform float displacement_factor = 0.0f;
uniform bool has_displacement_map = false;

uniform sampler2D displacement_map_sampler;

void share()
{
	position_modelspace = position;
	texcoord_modelspace = texcoord;
	normal_modelspace = normal;
	if(has_displacement_map)
	    position_modelspace += normal * texture2D(displacement_map_sampler, texcoord_modelspace).r * displacement_factor;

	model_matrix = m;
    view_matrix = v;
    projection_matrix = p;

    vec3 position_cameraspace = (view_matrix * model_matrix * vec4(position_modelspace, 1.0)).xyz;
    eye_direction_cameraspace = vec3(0, 0, 0) - position_cameraspace;

    // edit this as you wish
    const vec3 light_position_cameraspace = vec3(0, 0, 0);
    light_direction_cameraspace = light_position_cameraspace + eye_direction_cameraspace;
	
	vec3 bitangent = cross(tangent, normal);
	
	vec3 normal_cameraspace = normalize((v * m * vec4(normal, 0.0)).xyz);
	vec3 tangent_cameraspace = normalize((v * m * vec4(tangent, 0.0)).xyz);
	vec3 bitangent_cameraspace = normalize((v * m * vec4(bitangent, 0.0)).xyz);
	
	// Gramm-Schmidt Process
	tangent_cameraspace = normalize(tangent_cameraspace - dot(tangent_cameraspace, normal_cameraspace) * normal_cameraspace);
	
	tbn_matrix = transpose(mat3(tangent_cameraspace, bitangent_cameraspace, normal_cameraspace));
}

void main()
{
	share();
	gl_Position = (p * v * m) * vec4(position_modelspace, 1.0);
}