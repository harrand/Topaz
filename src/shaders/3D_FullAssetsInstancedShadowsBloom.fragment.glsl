// Fragment Shader version 4.30
#version 430

in vec3 position_modelspace;
in vec2 texcoord_modelspace;
in vec3 normal_modelspace;
in vec3 eye_direction_cameraspace;
in vec3 light_direction_cameraspace;

in mat4 model_matrix;
in mat4 view_matrix;
in mat3 tbn_matrix;
in vec4 position_lightspace;

uniform sampler2D texture_sampler;
uniform sampler2D normal_map_sampler;
uniform sampler2D parallax_map_sampler;
uniform sampler2D depth_map_sampler;
uniform sampler2D specular_map_sampler;
uniform sampler2D emissive_map_sampler;

uniform sampler2D extra_texture_sampler0;
uniform bool extra_texture0_exists = false;
uniform sampler2D extra_texture_sampler1;
uniform bool extra_texture1_exists = false;
uniform sampler2D extra_texture_sampler2;
uniform bool extra_texture2_exists = false;
uniform sampler2D extra_texture_sampler3;
uniform bool extra_texture3_exists = false;
uniform sampler2D extra_texture_sampler4;
uniform bool extra_texture4_exists = false;
uniform sampler2D extra_texture_sampler5;
uniform bool extra_texture5_exists = false;
uniform sampler2D extra_texture_sampler6;
uniform bool extra_texture6_exists = false;
uniform sampler2D extra_texture_sampler7;
uniform bool extra_texture7_exists = false;

uniform bool has_normal_map = false;
uniform bool has_parallax_map = false;
uniform bool has_specular_map = false;
uniform bool has_emissive_map = false;

uniform float parallax_multiplier;
uniform float parallax_bias;

layout(location = 0) out vec4 fragment_colour;
layout(location = 1) out vec4 bright_colour;

// This shader works primarily in camera-space.

struct DirectionalLight
{
	// Direction is in cameraspace.
	vec3 direction;
	vec3 colour;
	float power;
};

// Unlike hard-coded DirectionalLight, we expect the attributes to be in worldspace, NOT cameraspace.
const uint num_directional_lights = 8;
uniform DirectionalLight directional_lights[num_directional_lights];

struct PointLight
{
	// Position is in cameraspace.
	vec3 position;
	vec3 colour;
	float power;
};

const uint num_point_lights = 8;
uniform PointLight point_lights[num_point_lights];

#include "utility.header.glsl"

vec3 full_texture_colour(vec2 texcoord)
{
	vec3 texture_colour = texture(texture_sampler, texcoord).xyz;
	if(extra_texture0_exists)
		texture_colour += texture(extra_texture_sampler0, texcoord).xyz;
	if(extra_texture1_exists)
		texture_colour += texture(extra_texture_sampler1, texcoord).xyz;
	if(extra_texture2_exists)
		texture_colour += texture(extra_texture_sampler2, texcoord).xyz;
	if(extra_texture3_exists)
		texture_colour += texture(extra_texture_sampler3, texcoord).xyz;
	if(extra_texture4_exists)
		texture_colour += texture(extra_texture_sampler4, texcoord).xyz;
	if(extra_texture5_exists)
		texture_colour += texture(extra_texture_sampler5, texcoord).xyz;
	if(extra_texture6_exists)
		texture_colour += texture(extra_texture_sampler6, texcoord).xyz;
	if(extra_texture7_exists)
		texture_colour += texture(extra_texture_sampler7, texcoord).xyz;
	return texture_colour;
}

bool in_shadow()
{
	vec3 projection_coords = position_lightspace.xyz / position_lightspace.w;
	projection_coords = projection_coords * 0.5f + 0.5f;
	if(!in_range(projection_coords.x) || !in_range(projection_coords.y))
		return false;
	float closest_depth = texture(depth_map_sampler, projection_coords.xy).r;
	float current_depth = projection_coords.z;
	const float bias = 0.005f;
	return current_depth - bias > closest_depth;
}

void main()
{
	vec2 parallaxed_texcoord;
	if(has_parallax_map)
		parallaxed_texcoord = parallax_offset(texcoord_modelspace);
	else
		parallaxed_texcoord = texcoord_modelspace;
	// TBN matrix goes from cameraspace to tangentspace
	vec3 position_cameraspace = (view_matrix * model_matrix * vec4(position_modelspace, 1.0)).xyz;
	vec3 normal_cameraspace;
	if(has_normal_map)
		normal_cameraspace = transpose(tbn_matrix) * (texture(normal_map_sampler, parallaxed_texcoord).xyz * 255.0/128.0 - 1);
	else
		normal_cameraspace = normalize((view_matrix * model_matrix * vec4(normal_modelspace, 0.0)).xyz);
	vec3 texture_colour = full_texture_colour(parallaxed_texcoord);
	fragment_colour = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	 // Directional Component camera light. disabled by default.
	 /*
	DirectionalLight cam_light;
	cam_light.colour = vec3(1, 1, 1);
	cam_light.direction = vec3(0, 0, 0) + eye_direction_cameraspace;
	cam_light.power = 0.5f;
	vec3 light_direction_tangentspace = tbn_matrix * cam_light.direction;
	fragment_colour = vec4(diffuse_directional(cam_light, texture_colour, normal_cameraspace) + specular_directional(cam_light, texture_colour, normal_cameraspace), 1.0);
	*/
	for(uint i = 0; i < num_directional_lights; i++)
	{
		DirectionalLight light = directional_lights[i];
		// convert attribute(s) to cameraspace, then perform the processing.
		light.direction = (view_matrix * vec4(light.direction, 0.0)).xyz;
		fragment_colour += vec4(diffuse_directional(light, texture_colour, normal_cameraspace) + specular_directional(light, texture_colour, normal_cameraspace), 1.0);
	}
	for(uint i = 0; i < num_point_lights; i++)
	{
		PointLight light = point_lights[i];
		// convert attribute(s) to cameraspace, then perform the processing.
		light.position = (view_matrix * vec4(light.position, 1.0)).xyz;
		fragment_colour += vec4(diffuse(light, texture_colour, normal_cameraspace, position_cameraspace) + specular(light, texture_colour, normal_cameraspace, position_cameraspace), 1.0);
	}
	if(in_shadow())
		fragment_colour.xyz /= 2.0f;
	// For some reason, HDR textures have weird w-components (which i assume just fall low as hell, so ensure there is no transparency.)
	fragment_colour.w = 1.0f;
	const vec3 threshold = vec3(0.2126, 0.7152, 0.0722);
	float brightness = dot(fragment_colour.xyz, threshold);
	if(brightness > 1.0f)
		bright_colour = vec4(fragment_colour.xyz, 1);
	else
		bright_colour = vec4(0, 0, 0, 1);
	if(has_emissive_map)
	{
		vec4 emissive_colour = texture(emissive_map_sampler, parallaxed_texcoord);
		if(emissive_colour.a > 0.0)
		{
			fragment_colour = mix(fragment_colour, emissive_colour, vec4(0.5, 0.5, 0.5, 1.0));
			bright_colour = fragment_colour;
		}
	}
	/*
	// Non-Directional Component.
	PointLight cam_light;
	cam_light.colour = vec3(1, 1, 1);
	cam_light.position = vec3(0, 0, 0);
	cam_light.power = 20.0f;
	fragment_colour = vec4(diffuse(cam_light, texture_colour, normal_cameraspace, position_cameraspace) + specular(cam_light, texture_colour, normal_cameraspace, position_cameraspace), 1.0);
	*/
}