// Fragment Shader version 4.30
#version 430

in vec3 position_modelspace;
in vec2 texcoord_modelspace;

in mat4 model_matrix;
in mat4 view_matrix;

uniform sampler2D texture_sampler;
uniform uint shininess;

layout(location = 0) out vec4 fragment_colour;

void main()
{
	fragment_colour = texture(texture_sampler, texcoord_modelspace);
}