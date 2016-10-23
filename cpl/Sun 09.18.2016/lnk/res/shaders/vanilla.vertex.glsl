 // Vertex Shader version 4.30
#version 430

// Can be written from the cpu
attribute vec3 position;
attribute vec2 texcoord;
attribute vec3 normal;

varying vec2 texcoordShared;
varying vec3 normalShared;
varying vec3 positionShared;
varying mat4 viewShared;
// Lambertian Lighting is done per pixel, not per vertex, so send to fragment shader
// Can be changed/read by the cpu
//uniform mat4 mvp;
uniform mat4 m;
uniform mat4 v;
uniform mat4 p;

void main()
{
	// 1.0 because this position is a positional vector and not a directional vector (In which case it would be 0)
	gl_Position = (p * v * m) * vec4(position, 1.0);
	//gl_Position = vec4(position, 1.0);
	positionShared = (m * vec4(position, 1.0)).xyz;
	normalShared = (m * vec4(normal, 0.0)).xyz;
	texcoordShared = texcoord;
	viewShared = v;
}