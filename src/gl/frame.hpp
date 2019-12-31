#ifndef TOPAZ_GL_FRAME_HPP
#define TOPAZ_GL_FRAME_HPP
#include "glad/glad.h"

// Forward Declares.
class GLFWwindow;

namespace tz::gl
{
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
		 * Verify that the framebuffer is complete.
		 */
		virtual bool complete() const = 0;
		virtual bool operator==(GLuint handle) const = 0;
		virtual bool operator!=(GLuint handle) const = 0;
	private:
		unsigned int width;
		unsigned int height;
	};

	class Frame : public IFrame
	{
	public:
		Frame(unsigned int width, unsigned int height);
		~Frame();

		virtual void bind() const override;
		virtual bool complete() const override;
		virtual bool operator==(GLuint handle) const override;
		virtual bool operator!=(GLuint handle) const override;
	private:
		void verify() const;

		GLuint handle;
	};

	class WindowFrame : public IFrame
	{
	public:
		WindowFrame(GLFWwindow* handle);
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
}

#endif