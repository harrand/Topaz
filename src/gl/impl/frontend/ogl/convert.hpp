#ifndef TOPAZ_GL_IMPL_FRONTEND_OGL_CONVERT_HPP
#define TOPAZ_GL_IMPL_FRONTEND_OGL_CONVERT_HPP
#if TZ_OGL
#include "gl/declare/texture.hpp"
#include "gl/impl/backend/ogl/texture.hpp"

namespace tz::gl
{
    constexpr ogl::Texture::Format to_ogl(TextureFormat fmt);
    constexpr TextureFormat from_ogl(ogl::Texture::Format fmt);
}

#include "gl/impl/frontend/ogl/convert.inl"
#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_FRONTEND_VK_CONVERT_HPP