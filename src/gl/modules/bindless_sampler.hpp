#ifndef TOPAZ_GL_MODULES_BINDLESS_SAMPLER_HPP
#define TOPAZ_GL_MODULES_BINDLESS_SAMPLER_HPP
#include "gl/shader_preprocessor.hpp"

namespace tz::gl::p
{
	/**
	 * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
	 * @{
	 */
	/**
	 * \addtogroup tz_gl_p tz::gl Shader Preprocessing Module (TZGLP)
	 * @{
	 */

	/**
	 * Provides the custom Topaz PDT 'tz_bindless_sampler' define in GLSL. Is just a sampler2D with explicit intent to be used in an UBO/SSBO as a bindless sampler.
	 * 
	 */
	class BindlessSamplerModule : public IModule
	{
	public:
		/**
		 * Construct a BindlessSamplerModule, transforming all instances of 'tz_bindless_sampler' => 'sampler2D'.
		 */
		BindlessSamplerModule() = default;
		/**
		 * Invoke the module on the given source
		 * 
		 * @param source GLSL source-code to preprocess. Note that it doesn't have to be valid code.
		 */
		virtual void operator()(std::string& source) const override;
	private:
		std::string path;
	};

	/**
	 * @}
	 */
	/**
	 * @}
	 */
}

#endif // TOPAZ_GL_MODULES_INCLUDE_HPP