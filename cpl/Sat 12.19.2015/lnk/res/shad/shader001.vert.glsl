 // Vertex Shader version 4.30
#version 430

attribute vec3 pos;

void main()
{
	// 1.0 because this position is a positional vector and not a directional vector (In which case it would be 0)
	gl_Position = vec4(pos, 1.0);
}