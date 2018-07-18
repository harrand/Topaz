#version 430

in vec3 vs_position_modelspace;
in vec2 vs_texcoord_modelspace;
in vec3 vs_normal_modelspace;

uniform vec4 colour = vec4(1.0, 1.0, 1.0, 1.0);
uniform vec4 background_colour = vec4(1.0, 1.0, 1.0, 1.0);
uniform bool has_texture = false;
uniform bool has_background_colour = false;
uniform sampler2D texture_sampler;
uniform float exposure = 0.01;
uniform float gamma = 2.2f;

layout(location = 0) out vec4 fragment_colour;

void main()
{
	if(has_texture)
	{
		vec3 hdr_colour = texture2D(texture_sampler, vs_texcoord_modelspace).rgb;
		// perform reinhard tone mapping
        //vec3 mapped = hdr_colour / (hdr_colour + vec3(1.0));
        // perform exposure tone mapping
        vec3 mapped = vec3(1.0) - exp(-hdr_colour * exposure);
        // gamma correction
        mapped = pow(mapped, vec3(1.0 / gamma));
        fragment_colour = vec4(mapped, 1.0);
	}
	else
		fragment_colour = colour;
	if(has_background_colour && fragment_colour.w < 0.1)
		fragment_colour = background_colour;

}