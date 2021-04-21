#version 450
#extension GL_ARB_separate_shader_objects : enable
#pragma shader_stage(fragment)

layout(location = 0) out vec4 frag_colour;

void main()
{
    frag_colour = vec4(0.0, 1.0, 0.0, 1.0);
}