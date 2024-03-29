#ifndef TOPAZ_GL_IMPL_BACKEND_OGL2_IMAGE_HPP
#define TOPAZ_GL_IMPL_BACKEND_OGL2_IMAGE_HPP
#if TZ_OGL
#include "tz/core/data/vector.hpp"
#include "tz/gl/impl/opengl/detail/tz_opengl.hpp"
#include "tz/gl/impl/opengl/detail/sampler.hpp"
#include "tz/gl/impl/opengl/detail/image_format.hpp"
#include <optional>

namespace tz::gl::ogl2
{
	/**
	 * @ingroup tz_gl_ogl2_image
	 * Specifies creation flags for a @ref image.
	 */
	struct image_info
	{
		/// Format of the image data.
		image_format format;
		/// {Width, Height} of the image, in pixels.
		tz::vec2ui dimensions;
		/// Specifies how the image should be sampled in a shader.
		sampler shader_sampler;
	};

	/**
	 * @ingroup tz_gl_ogl2_image
	 * Documentation for OpenGL images.
	 */
	class image
	{
	public:
		/// Opaque handle for a bindless texture.
		using bindless_handle = GLuint64;
		/**
		 * Create a new image.
		 * @param info Specifies creation flags for the image.
		 */
		image(image_info info);
		image(const image& copy) = delete;
		image(image&& move);
		~image();
		image& operator=(const image& rhs) = delete;
		image& operator=(image&& rhs);

		/**
		 * @return image_format of the image's data.
		 */
		image_format get_format() const;
		/**
		 * @return {width, height} of the image, in pixels.
		 */
		tz::vec2ui get_dimensions() const;
		/**
		 * Retrieves the state specifying how the image is sampled in a shader.
		 */
		const sampler& get_sampler() const;
		/**
		 * Set the image data.
		 * @param texture_data View into bytes representing the data. Should match the format and dimensions of this texture, as an array of rows of pixel data.
		 */
		void set_data(std::span<const std::byte> texture_data);
		/**
		 * Make the image bindless, allowing for an alternate way to reference the image as a shader resource, via a bindless handle. See @ref image::get_bindless_handle() for usage. Once the image is made bindless, the action cannot be undone.
		 * @pre image cannot already be bindless, otherwise the behaviour is undefined (this is also a major hazard and may cause a GPU crash).
		 */
		void make_bindless();
		/**
		 * Query as to whether this image is bindless.
		 * @return True if the image is bindless, otherwise false.
		 */
		bool is_bindless() const;
		/**
		 * Retrieves the bindless texture handle for this bindless image.
		 * @pre image must be bindless, otherwise the behaviour is undefined.
		 * @return Bindless image handle.
		 */
		bindless_handle get_bindless_handle() const;

		using NativeType = GLuint;
		NativeType native() const;

		std::string debug_get_name() const;
		void debug_set_name(std::string name);

		/**
		 * Create an image which acts as a null image, that is, no operations are valid on it.
		 * @return Null image.
		 */
		static image null();
		/**
		 * Query as to whether the image is a null image. A null image is equivalent to @ref image::null().
		 */
		bool is_null() const;
	private:
		image();

		GLuint img;
		image_info info;
		std::optional<bindless_handle> maybe_bindless_handle;
		std::string debug_name = "";
	};

	namespace image_helper
	{
		void copy(const image& source, image& destination);
		image clone_resized(const image& image, tz::vec2ui new_size);
	}
}

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_BACKEND_OGL2_IMAGE_HPP
