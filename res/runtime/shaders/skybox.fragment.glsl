#version 430

layout(location = 0) out vec4 fragColor;

in vec3 cubePosition_modelspace;
uniform samplerCube cubeMapSampler;

void main()
{
	fragColor = texture(cubeMapSampler, cubePosition_modelspace);
}