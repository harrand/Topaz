#version 450
#pragma shader_stage(vertex)

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 colour;

layout(location = 0) out vec3 frag_colour;

void main()
{
    gl_Position = vec4(position, 0.0, 1.0);
    frag_colour = colour;
}