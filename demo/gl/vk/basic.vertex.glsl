#version 450
#pragma shader_stage(vertex)

layout(set = 0, binding = 0) uniform MVP
{
    mat4 m;
    mat4 v;
    mat4 p;
} mvp;

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 colour;

layout(location = 0) out vec3 frag_colour;

void main()
{
    mat4 mvp_matrix = mvp.p * mvp.v * mvp.m;
    gl_Position = mvp_matrix * vec4(position, 0.0, 1.0);
    frag_colour = colour;
}