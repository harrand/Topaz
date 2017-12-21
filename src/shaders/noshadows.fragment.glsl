// Fragment Shader version 4.30
#version 430

in vec3 position_modelspace;
in vec2 texcoord_modelspace;

in mat4 model_matrix;
in mat4 view_matrix;
in mat3 tbn_matrix;

uniform sampler2D texture_sampler;
uniform sampler2D normal_map_sampler;
uniform sampler2D parallax_map_sampler;

uniform uint shininess;
uniform float parallax_multiplier;
uniform float parallax_bias;

layout(location = 0) out vec4 fragment_colour;

const uint MAX_LIGHTS = 8u;

struct Light
{
	vec3 pos;
	vec3 colour;
	float power;
	float diffuse_component;
	float specular_component;
};

uniform Light lights[MAX_LIGHTS];

vec2 parallax_offset(vec3 light_direction_tangentspace)
{
	return texcoord_modelspace + light_direction_tangentspace.xy * (texture2D(parallax_map_sampler, texcoord_modelspace).r * parallax_multiplier + parallax_bias);
}

vec4 diffuse_directional(Light l, vec3 position_worldspace, vec3 light_direction_worldspace, vec3 normal, vec4 texture_colour)
{
	float cos_theta = clamp(dot(normal, light_direction_worldspace), 0.0, 1.0);
	return l.diffuse_component * texture_colour * vec4(l.colour, 1) * l.power * cos_theta;
}

vec4 specular_directional(Light l, vec3 eye_direction_worldspace, vec3 light_direction_worldspace, vec3 normal, vec4 texture_colour)
{
	vec3 towards_the_camera = normalize(eye_direction_worldspace);
	vec3 reflection_direction = reflect(-light_direction_worldspace, normal);
	float cos_alpha = clamp(dot(towards_the_camera, reflection_direction), 0, 1);
	return l.specular_component * texture_colour * vec4(l.colour, 1) * l.power * pow(cos_alpha, shininess);
}

vec4 specular(Light l, vec3 position_worldspace, vec3 eye_direction_worldspace, vec3 normal, vec4 texture_colour)
{
	vec3 light_direction = l.pos - position_worldspace;
	float distance = length(light_direction);
	return specular_directional(l, eye_direction_worldspace, light_direction, normal, texture_colour) / pow(distance, 2);
}

vec4 ambience(Light directional_light, vec4 texture_colour)
{
	float power = directional_light.power / 2.0;
	return texture_colour * vec4(power, power, power, 1);
}

void main()
{
	vec3 position_worldspace = (model_matrix * vec4(position_modelspace, 1.0)).xyz;
	vec3 position_cameraspace = (view_matrix * vec4(position_worldspace, 1.0)).xyz;

	vec3 eye_direction_cameraspace = vec3(0, 0, 0) - position_cameraspace;
	vec3 eye_direction_worldspace = (inverse(model_matrix) * vec4(eye_direction_cameraspace, 0.0)).xyz;
	vec3 light_direction_worldspace = vec3(1, 1, -1);
	vec3 light_direction_tangentspace = transpose(tbn_matrix) * (inverse(model_matrix) * vec4(light_direction_worldspace, 0.0)).xyz;
	vec4 texture_colour = texture(texture_sampler, parallax_offset(light_direction_tangentspace));
	
	const vec3 camera_position_cameraspace = vec3(0, 0, 0);
	const vec3 camera_position_worldspace = (inverse(view_matrix) * vec4(camera_position_cameraspace, 1.0)).xyz;
	vec3 normal = tbn_matrix * (texture(normal_map_sampler, texcoord_modelspace).xyz * 255.0/128.0 - 1);
	// tbn_matrix transforms tangent-space -> model-space
	// normal in model space, turn it to world space
	normal = normalize((model_matrix * vec4(normal, 0.0)).xyz);
	Light sun;
	sun.pos = -light_direction_worldspace;
	sun.colour = vec3(1, 1, 1);
	sun.power = 0.8;
	sun.diffuse_component = 1.0;
	sun.specular_component = 0.5;
	/*
	Light test_light;
	test_light.pos = vec3(0, 80, 50);
	test_light.colour = vec3(1, 0, 0);
	test_light.power = 5000;
	test_light.diffuse_component = 1.0;
	test_light.specular_component = 1.0;
	*/
	//fragment_colour = ambience(sun, texture_colour);
	fragment_colour = diffuse_directional(sun, position_worldspace, light_direction_worldspace, normal, texture_colour) + specular_directional(sun, eye_direction_worldspace, light_direction_worldspace, normal, texture_colour);
	//fragment_colour += specular(test_light, position_worldspace, eye_direction_cameraspace, normal, texture_colour);
	fragment_colour.w = 1.0f;
}