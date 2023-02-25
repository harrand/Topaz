#ifndef TOPAZ_GL_IMPL_BACKEND_OGL2_FRAMEBUFFER_HPP
#define TOPAZ_GL_IMPL_BACKEND_OGL2_FRAMEBUFFER_HPP
#if TZ_OGL
#include "tz/core/data/vector.hpp"
#include "tz/core/data/basic_list.hpp"
#include "tz/gl/impl/opengl/detail/tz_opengl.hpp"
#include "tz/gl/impl/opengl/detail/image.hpp"
#include "tz/gl/impl/opengl/detail/renderbuffer.hpp"
#include <variant>

namespace tz::gl::ogl2
{
	/**
	 * @ingroup tz_gl_ogl2_framebuffer
	 * Describes a reference to either an image or renderbuffer. Is used as a @ref framebuffer attachment.
	 */
	using framebuffer_texture = std::variant<const image*, const render_buffer*>;
	/**
	 * @ingroup tz_gl_ogl2_framebuffer
	 * Specifies creation flags for a @ref framebuffer.
	 */
	struct framebuffer_info
	{
		/// {width, height}, in pixels.
		tz::vec2ui dimensions;
		/// Depth attachment, if one exists. Default nullopt.
		std::optional<framebuffer_texture> maybe_depth_attachment = std::nullopt;
		/// List of colour attachments in-order. Default empty.
		tz::basic_list<framebuffer_texture> colour_attachments = {};
	};

	/**
	 * @ingroup tz_gl_ogl2_framebuffer
	 * Represents an OpenGL framebuffer object.
	 */
	class framebuffer
	{
	public:
		/**
		 * Create a new framebuffer.
		 * @param info Creation flags for the framebuffer.
		 */
		framebuffer(framebuffer_info info);
		framebuffer(const framebuffer& copy) = delete;
		framebuffer(framebuffer&& move);
		~framebuffer();
		
		framebuffer& operator=(const framebuffer& rhs) = delete;
		framebuffer& operator=(framebuffer&& rhs);
		
		/**
		 * Query as to whether the framebuffer has a depth attachment. If the framebuffer is a null framebuffer, then this will return whether we have enabled depth testing or not.
		 * @return True if depth attachment exists, false otherwise.
		 */
		bool has_depth_attachment() const;
		/**
		 * Retrieve the number of colour attachments associated with the framebuffer.
		 * @return number of colour attachments.
		 */
		unsigned int colour_attachment_count() const;
		/**
		 * Retrieve the dimensions {width, height}, in pixels, of every framebuffer attachment.
		 * @note Every attachment must have size matching this.
		 */
		tz::vec2ui get_dimensions() const;

		/**
		 * Bind the framebuffer, causing subsequent draw calls to render into the framebuffer instead of its previous target.
		 */
		void bind() const;
		/**
		 * Clear the framebuffer attachments, setting them to known values.
		 */
		void clear() const;

		/**
		 * Retrieve the null framebuffer. The null framebuffer represents the window (note that Topaz applications can only have a single window).
		 * @return Null framebuffer.
		 */
		static framebuffer null();
		/**
		 * Query as to whether this is the null framebuffer.
		 * @return True if this is the null framebuffer, otherwise false.
		 */
		bool is_null() const;

		using NativeType = GLuint;
		NativeType native() const;
	private:
		framebuffer(std::nullptr_t);

		GLuint fb;
		framebuffer_info info;
	};
}

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_BACKEND_OGL2_FRAMEBUFFER_HPP
