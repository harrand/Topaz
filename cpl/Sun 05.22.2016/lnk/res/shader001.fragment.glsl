// Fragment Shader version 4.30
#version 430

varying vec2 texcoord0;
varying vec3 normal0;
uniform sampler2D diffuse;

void main()
{
	// Output pixel colour. every pixel is red.
	// Both lD and normal0 need to be normalised.
	// Light shoots out from the z-axis, hardcoded example
	vec3 lightDirection = vec3(0, 0, -1);
	gl_FragColor = texture2D(diffuse, vec2(texcoord0.x, -texcoord0.y)) * (1*clamp(dot(-lightDirection, normal0), 0.0, 1.0));//vec4(1.0, 0.0, 1.0, 1.0);
}