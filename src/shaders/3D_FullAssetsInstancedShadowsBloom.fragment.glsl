// Fragment Shader version 4.30
#version 430

#include "lighting_utility.header.glsl"
#include "fragment_assert.header.glsl"
// This shader works primarily in camera-space.

in MeshData
{
    vec3 position_modelspace;
    vec4 position_lightspace;
    vec2 texcoord_modelspace;
    vec3 normal_modelspace;
} input_mesh_data;

in CameraData
{
    vec3 eye_direction_cameraspace;
    vec3 light_direction_cameraspace;
} input_camera_data;

in MatrixBlock
{
    mat4 model;
    mat4 view;
    mat4 projection;
    mat3 tbn;
} input_matrices;

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

// Unlike hard-coded DirectionalLight, we expect the attributes to be in worldspace, NOT cameraspace.
#static_print("Max number of directional lights = 8")
const uint num_directional_lights = 8;
uniform DirectionalLight directional_lights[num_directional_lights];

#static_print("Max number of point lights = 8")
const uint num_point_lights = 8;
uniform PointLight point_lights[num_point_lights];


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

void main()
{
	vec2 parallaxed_texcoord;
	if(has_parallax_map)
		parallaxed_texcoord = parallax_offset(normalize(input_matrices.tbn * input_camera_data.eye_direction_cameraspace), parallax_map_sampler, input_mesh_data.texcoord_modelspace, parallax_multiplier, parallax_bias);
	else
		parallaxed_texcoord = input_mesh_data.texcoord_modelspace;
	// TBN matrix goes from cameraspace to tangentspace
	vec3 position_cameraspace = (input_matrices.view * input_matrices.model * vec4(input_mesh_data.position_modelspace, 1.0)).xyz;
	vec3 normal_cameraspace;
	if(has_normal_map)
		normal_cameraspace = transpose(input_matrices.tbn) * (texture(normal_map_sampler, parallaxed_texcoord).xyz * 255.0/128.0 - 1);
	else
		normal_cameraspace = normalize((input_matrices.view * input_matrices.model * vec4(input_mesh_data.normal_modelspace, 0.0)).xyz);
	vec3 texture_colour = full_texture_colour(parallaxed_texcoord);
	fragment_colour = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	for(uint i = 0; i < num_directional_lights; i++)
	{
		DirectionalLight light = directional_lights[i];
		// convert attribute(s) to cameraspace, then perform the processing.
		light.direction = (input_matrices.view * vec4(light.direction, 0.0)).xyz;
		fragment_colour += vec4(diffuse_directional(light, texture_colour, normal_cameraspace) + specular_directional(light, texture_colour, normal_cameraspace, input_camera_data.eye_direction_cameraspace, OptionalSampler(has_specular_map, specular_map_sampler), input_mesh_data.texcoord_modelspace), 1.0);
	}
	for(uint i = 0; i < num_point_lights; i++)
	{
		PointLight light = point_lights[i];
		// convert attribute(s) to cameraspace, then perform the processing.
		light.position = (input_matrices.view * vec4(light.position, 1.0)).xyz;
		fragment_colour += vec4(diffuse(light, texture_colour, normal_cameraspace, position_cameraspace, input_camera_data.eye_direction_cameraspace) + specular(light, texture_colour, normal_cameraspace, position_cameraspace, input_camera_data.eye_direction_cameraspace, OptionalSampler(has_specular_map, specular_map_sampler), input_mesh_data.texcoord_modelspace), 1.0);
	}
	if(in_shadow(input_mesh_data.position_lightspace, depth_map_sampler))
		fragment_colour.xyz /= 2.0f;
	// For some reason, HDR textures have weird w-components (which i assume just fall low as hell, so ensure there is no transparency.)
	fragment_colour.w = 1.0f;
	#static_print("Shadowing threshold == vec3(0.2126, 0.7152, 0.0722)")
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
}