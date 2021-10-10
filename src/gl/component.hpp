#ifndef TOPAZ_GL_COMPONENT_HPP
#define TOPAZ_GL_COMPONENT_HPP

#if TZ_VULKAN
#include "gl/impl/frontend/vk/component.hpp"
namespace tz::gl
{
	using BufferComponent = BufferComponentVulkan;
	using TextureComponent = TextureComponentVulkan;
}
#elif TZ_OGL
#include "gl/impl/frontend/ogl/component.hpp"
namespace tz::gl
{
	using BufferComponent = BufferComponentOGL;
	using TextureComponent = TextureComponentOGL;
}
#endif

#endif // TOPAZ_GL_RENDERER_HPP