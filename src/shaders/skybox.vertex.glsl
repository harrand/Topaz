#version 430

layout(location = 0) in vec3 position;
out vec3 vs_cube_position_modelspace;

uniform mat4 m;
uniform mat4 v;
uniform mat4 p;

void main()
{
	gl_Position = (p * v * m) * vec4(position, 1.0);
	gl_Position.z = gl_Position.w - 1; // Fix to far plane.
	vs_cube_position_modelspace = position;
}