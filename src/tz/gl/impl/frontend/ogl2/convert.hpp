#ifndef TOPAZ_GL_IMPL_FRONTEND_OGL2_CONVERT_HPP
#define TOPAZ_GL_IMPL_FRONTEND_OGL2_CONVERT_HPP
#if TZ_OGL
#include "tz/gl/declare/image_format.hpp"
#include "tz/gl/impl/backend/ogl2/image_format.hpp"

namespace tz::gl
{
	constexpr tz::gl::ogl2::ImageFormat to_ogl2(ImageFormat fmt);
	constexpr ImageFormat from_ogl2(tz::gl::ogl2::ImageFormat fmt);
}
#include "tz/gl/impl/frontend/ogl2/convert.inl"

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_FRONTEND_OGL2_CONVERT_HPP
