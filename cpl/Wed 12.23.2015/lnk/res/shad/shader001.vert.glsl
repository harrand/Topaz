 // Vertex Shader version 4.30
#version 430

// Can be written from the cpu
attribute vec3 pos;
attribute vec2 texcoord;

varying vec2 texcoord0;

// Can be changed/read by the cpu
uniform mat4 transform;

void main()
{
	// 1.0 because this position is a positional vector and not a directional vector (In which case it would be 0)
	gl_Position = transform * vec4(pos, 1.0);
	texcoord0 = texcoord;
}