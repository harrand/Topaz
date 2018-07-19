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

uniform sampler2D texture_sampler;
uniform sampler2D normal_map_sampler;
uniform sampler2D parallax_map_sampler;

uniform bool has_normal_map = false;
uniform bool has_parallax_map = false;

uniform float parallax_multiplier;
uniform float parallax_bias;

layout(location = 0) out vec4 fragment_colour;

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

vec3 diffuse_directional(DirectionalLight light, vec3 diffuse_colour, vec3 normal_cameraspace)
{
    float cos_theta = clamp(dot(normal_cameraspace, light.direction), 0.0, 1.0);
    return diffuse_colour * light.colour * light.power * cos_theta;
}

vec3 diffuse(PointLight light, vec3 diffuse_colour, vec3 normal_cameraspace, vec3 position_cameraspace)
{
    float distance = length(light.position - position_cameraspace);
    DirectionalLight directional;
    directional.direction = light.position + eye_direction_cameraspace;
    directional.colour = light.colour;
    directional.power = light.power;
    return diffuse_directional(directional, diffuse_colour, normal_cameraspace) / pow(distance, 2);
}

vec3 specular_directional(DirectionalLight light, vec3 specular_colour, vec3 normal_cameraspace)
{
    /* //phong specular
    vec3 reflection = reflect(-light.direction, normal_cameraspace);
    float cos_alpha = clamp(dot(eye_direction_cameraspace, reflection), 0.0, 1.0);
    return specular_colour * light.colour * light.power * pow(cos_alpha, 5);
    */

    //blinn-phong specular
    const float shininess = 5;
    vec3 halfway_direction = normalize(light.direction + eye_direction_cameraspace);
    float specular_component = pow(max(dot(normal_cameraspace, halfway_direction), 0.0), shininess);
    return specular_colour * light.colour * light.power * specular_component;
}

vec3 specular(PointLight light, vec3 specular_colour, vec3 normal_cameraspace, vec3 position_cameraspace)
{
    float distance = length(light.position - position_cameraspace);
    DirectionalLight directional;
    directional.direction = light.position + eye_direction_cameraspace;
    directional.colour = light.colour;
    directional.power = light.power;
    return specular_directional(directional, specular_colour, normal_cameraspace) / pow(distance, 2);
}

vec2 parallax_offset(vec2 texcoord)
{
    // MUST normalize this, or everything goes a bit weird.
    vec3 eye_direction_tangentspace = normalize(tbn_matrix * eye_direction_cameraspace);
    vec2 texcoord_offset = eye_direction_tangentspace.xy * (texture2D(parallax_map_sampler, texcoord).r * parallax_multiplier + parallax_bias);
    texcoord_offset.y = -texcoord_offset.y;
	return texcoord_modelspace + texcoord_offset;
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
	 // Directional Component.
    DirectionalLight cam_light;
    cam_light.colour = vec3(1, 1, 1);
    cam_light.direction = vec3(0, 0, 0) + eye_direction_cameraspace;
    cam_light.power = 0.5f;
    vec3 light_direction_tangentspace = tbn_matrix * cam_light.direction;
    vec3 texture_colour = texture(texture_sampler, parallaxed_texcoord).xyz;
    fragment_colour = vec4(diffuse_directional(cam_light, texture_colour, normal_cameraspace) + specular_directional(cam_light, texture_colour, normal_cameraspace), 1.0);
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
	/*
    // Non-Directional Component.
    PointLight cam_light;
    cam_light.colour = vec3(1, 1, 1);
    cam_light.position = vec3(0, 0, 0);
    cam_light.power = 20.0f;
    fragment_colour = vec4(diffuse(cam_light, texture_colour, normal_cameraspace, position_cameraspace) + specular(cam_light, texture_colour, normal_cameraspace, position_cameraspace), 1.0);
    */
}