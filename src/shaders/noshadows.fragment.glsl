// Fragment Shader version 4.30
#version 430

in vec3 position_modelspace;
in vec3 cube_position_modelspace;
in vec2 texcoord_modelspace;
in vec3 normal_modelspace;

in mat4 model_matrix;
in mat4 view_matrix;
in mat3 tbn_matrix;

uniform sampler2D texture_sampler;
uniform sampler2D normal_map_sampler;
uniform sampler2D parallax_map_sampler;

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

const vec3 position_worldspace = (model_matrix * vec4(position_modelspace, 1.0)).xyz;
const vec3 position_cameraspace = (view_matrix * vec4(position_worldspace, 1.0)).xyz;

const vec3 camera_position_cameraspace = vec3(0, 0, 0);
vec3 camera_position_worldspace = (inverse(view_matrix) * vec4(camera_position_cameraspace, 1.0)).xyz;
const vec3 eye_direction_cameraspace = vec3(0, 0, 0) - position_cameraspace;
const vec3 eye_direction_tangentspace = tbn_matrix * eye_direction_cameraspace;

const vec3 ld_cameraspace = camera_position_cameraspace - position_cameraspace;
const float distance = length(ld_cameraspace);
const vec3 light_direction_cameraspace = normalize(ld_cameraspace);
const vec3 light_direction_tangentspace = normalize(tbn_matrix * light_direction_cameraspace);

vec2 getTexcoordOffset()
{
	return texcoord_modelspace + light_direction_tangentspace.xy * (texture2D(parallax_map_sampler, texcoord_modelspace).r * parallax_multiplier + parallax_bias);
}

vec4 texture_colour = texture2D(texture_sampler, texcoord_modelspace);//texture2D(texture_sampler, getTexcoordOffset());

vec4 getDiffuseComponent(Light l, vec3 parsed_normal_tangentspace)
{
	vec3 light_position_cameraspace = (view_matrix * vec4(l.pos, 1.0)).xyz;
	vec3 displacement_from_light = light_position_cameraspace - position_cameraspace;
	float cos_theta = clamp(dot(parsed_normal_tangentspace, tbn_matrix * displacement_from_light), 0.0, 1.0);
	float distance_from_light = length(displacement_from_light);
	return l.diffuse_component * texture_colour * vec4(l.colour, 1) * l.power * cos_theta / (distance_from_light * distance_from_light);
}

vec4 getAmbientComponent()
{
	return texture_colour * vec4(0.01, 0.01, 0.01, 1);
}

vec4 getSpecularComponent(Light l, vec3 parsed_normal_tangentspace)
{
	vec3 light_position_cameraspace = (view_matrix * vec4(l.pos, 1.0)).xyz;
	vec3 displacement_from_light = light_position_cameraspace - position_cameraspace;
	vec3 E = normalize(eye_direction_tangentspace);
	vec3 R = reflect(-normalize(tbn_matrix * displacement_from_light), parsed_normal_tangentspace);
	float cos_alpha = clamp(dot(E, R), 0, 1);
	float distance_from_light = length(displacement_from_light);
	return l.specular_component * texture_colour * vec4(l.colour, 1) * l.power * pow(cos_alpha, 5) / (distance_from_light * distance_from_light);
}

void main()
{
	vec3 normal_tangentspace = normalize(texture2D(normal_map_sampler, getTexcoordOffset()).xyz * 255.0/128.0 - 1);
	fragment_colour = vec4(0, 0, 0, 0);
	Light camera_light;
	camera_light.pos = camera_position_worldspace;
	camera_light.colour = vec3(1, 1, 1);
	camera_light.power = 1000;
	camera_light.diffuse_component = 1.0;
	camera_light.specular_component = 1.0;
	fragment_colour += getAmbientComponent() + getDiffuseComponent(camera_light, normal_tangentspace) + getSpecularComponent(camera_light, normal_tangentspace);
	for(uint i = 0u; i < MAX_LIGHTS; i++)
		fragment_colour += getDiffuseComponent(lights[i], normal_tangentspace) + getSpecularComponent(lights[i], normal_tangentspace);
}