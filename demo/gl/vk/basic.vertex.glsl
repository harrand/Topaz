#version 450
#pragma shader_stage(vertex)

layout(set = 0, binding = 0) uniform MVP
{
    mat4 m;
    mat4 v;
    mat4 p;
} mvp;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 colour;
layout(location = 2) in vec2 tex_coord;

layout(location = 0) out vec3 frag_colour;
layout(location = 1) out vec2 frag_tex_coord;

void main()
{
    mat4 mvp_matrix = mvp.p * mvp.v * mvp.m;
    gl_Position = mvp_matrix * vec4(position, 1.0);
    frag_colour = colour;
    frag_tex_coord = tex_coord;
}