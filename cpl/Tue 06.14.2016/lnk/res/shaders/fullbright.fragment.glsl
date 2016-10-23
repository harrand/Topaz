// Fragment Shader version 4.30
#version 430

varying vec2 texcoord0;
//varying vec3 normal0;
varying vec3 position0;
varying mat4 v0;
uniform sampler2D diffuse;

void main()
{
	// Output pixel colour. every pixel is red.
	// Both lD and normal0 need to be normalised.
	// Light shoots out from the z-axis, hardcoded example
	// need about 6 light sources
	gl_FragColor = texture2D(diffuse, texcoord0);
}