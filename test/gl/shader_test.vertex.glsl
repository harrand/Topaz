#pragma shader_stage(vertex)
#extension GL_KHR_vulkan_glsl : enable

#if TZ_VULKAN
    int tz_vertex_id = gl_VertexIndex;
#elif TZ_OGL
    int tz_vertex_id = gl_VertexID;
#endif

resource(id = 0) const buffer UBO
{
    int data[5];
} ubo;

resource(id = 1) buffer SSBO
{
    int data[5];
} ssbo;

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

void main()
{
    gl_Position = vec4(positions[tz_vertex_id], 0.0, 1.0);
}