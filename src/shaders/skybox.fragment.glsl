#version 430

layout(location = 0) out vec4 fragColor;

in vec3 vs_cube_position_modelspace;
uniform samplerCube cube_map_sampler;

void main()
{
	fragColor = texture(cube_map_sampler, vs_cube_position_modelspace);
}