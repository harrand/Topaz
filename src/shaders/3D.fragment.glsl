// Fragment Shader version 4.30
#version 430

in vec2 texcoord_modelspace;

uniform sampler2D texture_sampler;

layout(location = 0) out vec4 fragment_colour;

void main()
{
	fragment_colour = texture(texture_sampler, texcoord_modelspace);
}