#version 430

in vec2 texture_coordinate;

layout(location = 0) out vec4 fragment_colour;

uniform vec4 colour;
uniform bool has_texture;
uniform sampler2D texture_sampler;

void main()
{
	if(has_texture)
		fragment_colour = texture2D(texture_sampler, texture_coordinate);
	else
		fragment_colour = colour;
}