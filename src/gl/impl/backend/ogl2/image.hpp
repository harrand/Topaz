#ifndef TOPAZ_GL_IMPL_BACKEND_OGL2_IMAGE_HPP
#define TOPAZ_GL_IMPL_BACKEND_OGL2_IMAGE_HPP
#if TZ_OGL
#include "core/vector.hpp"
#include "gl/impl/backend/ogl2/tz_opengl.hpp"
#include "gl/impl/backend/ogl2/sampler.hpp"
#include "gl/impl/backend/ogl2/image_format.hpp"

namespace tz::gl::ogl2
{
	/**
	 * @ingroup tz_gl_ogl2_image
	 * Specifies creation flags for a @ref Image.
	 */
	struct ImageInfo
	{
		/// Format of the image data.
		ImageFormat format;
		/// {Width, Height} of the image, in pixels.
		tz::Vec2ui dimensions;
		/// Specifies how the image should be sampled in a shader.
		Sampler sampler;
	};

	/**
	 * @ingroup tz_gl_ogl2_image
	 * Documentation for OpenGL Images.
	 */
	class Image
	{
	public:
		/// Opaque handle for a bindless texture.
		using BindlessTextureHandle = GLuint64;
		/**
		 * Create a new Image.
		 * @param info Specifies creation flags for the image.
		 */
		Image(ImageInfo info);
		Image(const Image& copy) = delete;
		Image(Image&& move);
		~Image();
		Image& operator=(const Image& rhs) = delete;
		Image& operator=(Image&& rhs);

		/**
		 * @return ImageFormat of the image's data.
		 */
		ImageFormat get_format() const;
		/**
		 * @return {width, height} of the image, in pixels.
		 */
		tz::Vec2ui get_dimensions() const;
		/**
		 * Retrieves the state specifying how the image is sampled in a shader.
		 */
		const Sampler& get_sampler() const;
	
		/**
		 * Make the image bindless, allowing for an alternate way to reference the image as a shader resource, via a bindless handle. See @ref Image::get_bindless_handle() for usage. Once the image is made bindless, the action cannot be undone.
		 * @pre Image cannot already be bindless, otherwise the behaviour is undefined (this is also a major hazard and may cause a GPU crash).
		 */
		void make_bindless();
		/**
		 * Query as to whether this image is bindless.
		 * @return True if the image is bindless, otherwise false.
		 */
		bool is_bindless() const;
		/**
		 * Retrieves the bindless texture handle for this bindless image.
		 * @pre Image must be bindless, otherwise the behaviour is undefined.
		 * @return Bindless image handle.
		 */
		BindlessTextureHandle get_bindless_handle() const;

		using NativeType = GLuint;
		NativeType native() const;

		/**
		 * Create an image which acts as a null image, that is, no operations are valid on it.
		 * @return Null Image.
		 */
		static Image null();
		/**
		 * Query as to whether the image is a null image. A null image is equivalent to @ref Image::null().
		 */
		bool is_null() const;
	private:
		Image();

		GLuint image;
		ImageInfo info;
		std::optional<BindlessTextureHandle> maybe_bindless_handle;
	};

	namespace image
	{
		void copy(const Image& source, Image& destination);
		Image clone_resized(const Image& image, tz::Vec2ui new_size);
	}
}

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_BACKEND_OGL2_IMAGE_HPP
