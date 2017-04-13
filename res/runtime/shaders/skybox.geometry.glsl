#version 430
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 vs_cubePosition_modelspace[3];

out vec3 cubePosition_modelspace;

void main()
{
	for(unsigned int i = 0; i < gl_in.length(); i++)
	{
		cubePosition_modelspace = vs_cubePosition_modelspace[i];
		gl_Position = gl_in[i].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}