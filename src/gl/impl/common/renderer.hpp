#ifndef TOPAZ_GL_IMPL_COMMON_RENDERER_HPP
#define TOPAZ_GL_IMPL_COMMON_RENDERER_HPP

namespace tz::gl
{
    enum class RendererInputFrequency
    {
        PerVertexBasis,
        PerInstanceBasis
    };

    enum class RendererComponentType
    {
        Float32,
        Float32x2,
        Float32x3
    };
}

#endif // TOPAZ_GL_IMPL_COMMON_RENDERER_HPP