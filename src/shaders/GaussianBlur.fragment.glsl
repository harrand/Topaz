#version 430

in vec3 vs_position_modelspace;
in vec2 vs_texcoord_modelspace;
in vec3 vs_normal_modelspace;

const int kernel_size = 5;

uniform sampler2D texture_sampler;
uniform bool horizontal = true;
uniform float weight[kernel_size] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

layout(location = 0) out vec4 fragment_colour;

void main()
{
    vec2 tex_offset = 1.0 / textureSize(texture_sampler, 0); // gets size of single texel
    vec3 result = texture(texture_sampler, vs_texcoord_modelspace).rgb * weight[0]; // current fragment's contribution
    if(horizontal)
    {
        for(int i = 0; i < kernel_size; i++)
        {
            result += texture(texture_sampler, vs_texcoord_modelspace + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(texture_sampler, vs_texcoord_modelspace - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 0; i < kernel_size; i++)
        {
           result += texture(texture_sampler, vs_texcoord_modelspace + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
           result += texture(texture_sampler, vs_texcoord_modelspace - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
    // then perform tone mapping...

    fragment_colour = vec4(result, 1.0);
    //fragment_colour = texture(texture_sampler, vs_texcoord_modelspace);
    //fragment_colour = vec4(1, 0, 0, 1);
}