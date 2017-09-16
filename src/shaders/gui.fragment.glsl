#version 430

in vec3 vs_position_modelspace;
in vec2 vs_texcoord_modelspace;
in vec3 vs_normal_modelspace;

uniform vec3 colour = vec3(1.0, 1.0, 1.0);
uniform vec3 background_colour = vec3(1.0, 1.0, 1.0);
uniform vec3 text_border_colour = vec3(0.0, 0.0, 0.0);
uniform bool has_texture = false;
uniform bool has_background_colour = false;
uniform bool has_text_border_colour = false;
uniform sampler2D texture_sampler;

layout(location = 0) out vec4 fragment_colour;

void main()
{
	if(has_texture)
		fragment_colour = texture2D(texture_sampler, vs_texcoord_modelspace);
	else
		fragment_colour = vec4(colour, 1.0);
	if(has_text_border_colour && fragment_colour.w > 0.3 && fragment_colour.w < 0.775)
		fragment_colour = vec4(text_border_colour, 1.0);
	if(has_background_colour && fragment_colour.w < (has_text_border_colour ? 0.3 : 0.9))
			fragment_colour = vec4(background_colour, 1.0);
}