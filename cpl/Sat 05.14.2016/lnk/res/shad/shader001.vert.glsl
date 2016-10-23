 // Vertex Shader version 4.30
#version 430

// Can be written from the cpu
attribute vec3 pos;
attribute vec2 texcoord;
attribute vec3 normal;

varying vec2 texcoord0;
varying vec3 normal0;
// Lambertian Lighting is done per pixel, not per vertex, so send to fragment shader
// Can be changed/read by the cpu
uniform mat4 transform;

void main()
{
	// 1.0 because this position is a positional vector and not a directional vector (In which case it would be 0)
	gl_Position = vec4(pos, 1.0);//transform * vec4(pos, 1.0);
	texcoord0 = texcoord;
	// Need to apply transformation to normal. Normal's a directional vector to parlast = 0
	// This method breaks down when there's a non-uniform scale on the mesh.
	// To fix, invert scale matrix and transpose.
	normal0 = (transform * vec4(normal, 0.0)).xyz;
}