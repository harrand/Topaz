#version 430
#extension GL_EXT_nonuniform_qualifier : require
#pragma shader_stage fragment
layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 fcol;

layout(binding = 1) uniform sampler2D tz_textures[];
#define sample(id, uv) texture(tz_textures[id], uv)
#define texsize(id) textureSize(tz_textures[id], 0)

void main()
{
	fcol = sample(0, uv);
}
