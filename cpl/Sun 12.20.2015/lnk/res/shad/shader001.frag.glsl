// Fragment Shader version 4.30
#version 430

varying vec2 texcoord0;
uniform sampler2D diffuse;

void main()
{
	// Output pixel colour. every pixel is red.
	gl_FragColor = texture2D(diffuse, texcoord0);//vec4(1.0, 0.0, 1.0, 1.0);
}