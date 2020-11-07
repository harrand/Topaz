#ifndef TOPAZ_GL_FRAME_HPP
#define TOPAZ_GL_FRAME_HPP
#include "glad/glad.h"
#include "gl/texture.hpp"
#include <deque>
#include <queue>
#include <variant>

// Forward Declares.
struct GLFWwindow;

namespace tz::gl
{
	/**
	 * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
	 * @{
	 */

	/**
	 * Interface for anything that can be a render-target.
	 * 
	 * This includes windows and framebuffers.
	 */
	class IFrame
	{
	public:
		/**
		 * Construct some render-target that has the given dimensions.
		 * @param width Width, in pixels.
		 * @param height Height, in pixels.
		 */
		IFrame(unsigned int width, unsigned int height);
		IFrame(const IFrame& copy) = delete;
		IFrame(IFrame&& move);

		IFrame& operator=(const IFrame& rhs) = delete;
		IFrame& operator=(IFrame&& rhs);
		virtual ~IFrame() = default;

		/**
		 * Get the current width.
		 * @return Width, in pixels.
		 */
		virtual unsigned int get_width() const;
		/**
		 * Get the current height.
		 * @return Height, in pixels.
		 */
		virtual unsigned int get_height() const;
		/**
		 * Set the current width.
		 * @param width Width, in pixels.
		 */
		virtual void set_width(unsigned int width);
		/**
		 * Set the current height.
		 * @param height Height, in pixels.
		 */
		virtual void set_height(unsigned int height);
		/**
		 * Make this frame the current render-target.
		 * 
		 * Note: This will make subsequent render invocations draw into this frame.
		 */
		virtual void bind() const = 0;
		/**
		 * Clear this frame's data.
		 */
		void clear() const;
		/**
		 * Set the colour of background pixels.
		 */
		void set_clear_color(float r, float g, float b);
		/**
		 * Verify that the framebuffer is complete.
		 */
		virtual bool complete() const = 0;
		virtual bool operator==(GLuint handle) const = 0;
		virtual bool operator!=(GLuint handle) const = 0;
	private:
		unsigned int width;
		unsigned int height;
	};

	/**
	 * Represents a framebuffer.
	 */
	class Frame : public IFrame
	{
	public:
		/**
		 * Construct a frame with the given width and height, in pixels.
		 */
		Frame(unsigned int width, unsigned int height);
		Frame(const Frame& copy) = delete;
		Frame(Frame&& move);
		Frame& operator=(const Frame& rhs) = delete;
		Frame& operator=(Frame&& rhs);
		~Frame();

		/**
		 * Bind the frame, causing subsequent render operations to output into this frame.
		 */
		virtual void bind() const override;
		/**
		 * Query as to whether the frame is in a state that it can be validly bound.
		 * 
		 * @return True if the frame can be bound. False otherwise.
		 */
		virtual bool complete() const override;
		/**
		 * Add a texture-like object to the given attachment in the frame. The object is constructed in-place.
		 * 
		 * Note: A "texture-like object" is either a tz::gl::Texture or tz::gl::RenderBuffer.
		 * @tparam TextureType Type of the texture-like object to construct. This must be either a tz::gl::Texture or tz::gl::RenderBuffer.
		 * @tparam Args Types of arguments used to construct the attachment in-place.
		 * @param attachment Description of the attachment that the texture-like object represents. Examples are GL_COLOR_ATTACHMENT0 or GL_DEPTH_ATTACHMENT.
		 * @param args Values of arguments used to construct the attachment in-place.
		 * @return Reference to the constructed texture-like object.
		 */
		template<class TextureType, typename... Args>
		TextureType& emplace(tz::gl::FrameAttachment attachment, Args&&... args);
		/**
		 * Add a tz::gl::Texture to the given attachment in the frame. The texture is constructed in-place.
		 * 
		 * @tparam Args Types of arguments used to construct the attachment in-place.
		 * @param attachment Description of the attachment that the texture represents. Examples are GL_COLOR_ATTACHMENT0 or GL_DEPTH_ATTACHMENT.
		 * @param args Values of arguments used to construct the attachment in-place.
		 * @return Reference to the constructed texture.
		 */
		template<typename... Args>
		Texture& emplace_texture(tz::gl::FrameAttachment attachment, Args&&... args);
		/**
		 * Add a tz::gl::RenderBuffer to the given attachment in the frame. The renderbuffer is constructed in-place.
		 * 
		 * @tparam Args Types of arguments used to construct the attachment in-place.
		 * @param attachment Description of the attachment that the renderbuffer represents. Examples are GL_COLOR_ATTACHMENT0 or GL_DEPTH_ATTACHMENT.
		 * @param args Values of arguments used to construct the attachment in-place.
		 * @return Reference to the constructed renderbuffer.
		 */
		template<typename... Args>
		RenderBuffer& emplace_renderbuffer(tz::gl::FrameAttachment attachment, Args&&... args);
		/**
		 * Set the output-attachment.
		 * 
		 * Note: At present, only one output is supported. This is likely to change soon.
		 * @param attachment Desired output attachment type.
		 */
		void set_output_attachment(tz::gl::FrameAttachment attachment) const;
		virtual bool operator==(GLuint handle) const override;
		virtual bool operator!=(GLuint handle) const override;
	private:
		void verify() const;
		void process_pending_attachments() const;

		GLuint handle;
		std::deque<std::pair<GLenum, std::variant<tz::gl::Texture, tz::gl::RenderBuffer>>> attachments;
		mutable std::queue<std::pair<GLenum, tz::gl::Texture*>> pending_attachments;
	};

	class WindowFrame : public IFrame
	{
	public:
		WindowFrame(GLFWwindow* handle);
		WindowFrame(const WindowFrame& copy) = delete;
		WindowFrame(WindowFrame&& move);

		WindowFrame& operator=(const WindowFrame& rhs) = delete;
		WindowFrame& operator=(WindowFrame&& rhs);
		~WindowFrame() = default;
		virtual void bind() const override;
		virtual bool complete() const override;
		virtual bool operator==(GLuint handle) const override;
		virtual bool operator!=(GLuint handle) const override;
	private:
		GLFWwindow* handle;
	};

	namespace bound
	{
		GLuint frame();
	}

	/**
	 * @}
	 */
}

#include "gl/frame.inl"
#endif