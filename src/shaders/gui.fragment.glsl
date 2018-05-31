#version 430

in vec3 vs_position_modelspace;
in vec2 vs_texcoord_modelspace;
in vec3 vs_normal_modelspace;

uniform vec4 colour = vec4(1.0, 1.0, 1.0, 1.0);
uniform vec4 background_colour = vec4(1.0, 1.0, 1.0, 1.0);
uniform bool has_texture = false;
uniform bool has_background_colour = false;
uniform sampler2D texture_sampler;

layout(location = 0) out vec4 fragment_colour;

void main()
{
	if(has_texture)
		fragment_colour = texture2D(texture_sampler, vs_texcoord_modelspace);
	else
		fragment_colour = colour;
	if(has_background_colour && fragment_colour.w < 0.1)
		fragment_colour = background_colour;

}