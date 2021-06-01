#version 450
#extension GL_ARB_separate_shader_objects : enable
#pragma shader_stage(fragment)

layout(binding = 1) uniform sampler2D tex_sampler;
layout(location = 0) in vec2 frag_tex_coord;

layout(location = 0) out vec4 output_colour;

void main()
{
    output_colour = texture(tex_sampler, frag_tex_coord);
}