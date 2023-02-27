#ifndef TOPAZ_GL_2_API_DEVICE_HPP
#define TOPAZ_GL_2_API_DEVICE_HPP
#include "tz/gl/declare/image_format.hpp"
#include "tz/gl/api/renderer.hpp"
#include "tz/gl/api/schedule.hpp"
#include <functional>
#include <type_traits>

namespace tz::gl
{
	/**
	 * @ingroup tz_gl2
	 * Named Requirement: device
	 * Implemented by @ref tz_gl2_device
	 *
	 * device types are types which:
	 * - Implement the factory pattern for various graphics library objects, such as @ref renderer_type
	 * - Have direct access to the window surface, and exposes relevant information about said surface.
	 */
	template<typename T, typename R>
	concept device_type = requires(T t, R& rinfo, renderer_handle h)
	{
		requires std::is_default_constructible_v<std::decay_t<T>>;
		{t.create_renderer(rinfo)} -> std::same_as<renderer_handle>;
		{t.destroy_renderer(h)} -> std::same_as<void>;
		{t.renderer_count()} -> std::convertible_to<std::size_t>;
		{t.get_renderer(h)} -> renderer_type;
		{t.get_window_format()} -> std::same_as<image_format>;
		{t.dbgui()} -> std::same_as<void>;
		{t.begin_frame()} -> std::same_as<void>;
		{t.end_frame()} -> std::same_as<void>;
	};

	template<renderer_type R>
	class device_common
	{
	public:
		device_common() = default;
		const R& get_renderer(tz::gl::renderer_handle handle) const;
		R& get_renderer(tz::gl::renderer_handle handle);
		void destroy_renderer(tz::gl::renderer_handle handle);
		std::size_t renderer_count() const;
		const tz::gl::schedule& render_graph() const;
		tz::gl::schedule& render_graph();
		// Derived needs to define create_renderer still. They can use emplace_renderer as a helper function.
	protected:
		tz::gl::renderer_handle emplace_renderer(const tz::gl::renderer_info& rinfo);
		void internal_clear();
		void post_add_renderer(std::size_t rid, const tz::gl::renderer_info& rinfo);
	private:
		std::vector<R> renderers;
		std::vector<std::size_t> free_list = {};
		tz::gl::schedule render_schedule = {};
	};

	template<tz::gl::device_type<tz::gl::renderer_info> T>
	void common_device_dbgui(T& device);

	#if TZ_VULKAN && TZ_OGL
	// Documentation only.
	/**
	 * @ingroup tz_gl2
	 * Implements @ref tz::gl::device_type
	 */
	class device
	{
	public:
		/**
		 * Create a new @ref tz::gl::renderer.
		 * @param rinfo Describes the intended behaviour of the renderer.
		 * @return Opaque handle associated with the newly-created renderer.
		 */
		tz::gl::renderer_handle create_renderer(tz::gl::renderer_info& rinfo);
		/**
		 * Destroy an existing renderer. The renderer will no longer exist.
		 * @pre The handle `rh` must have been the return value of a prior invocation of @ref create_renderer().
		 * @post The renderer associated with the handle `rh` must no longer be accessed. Any existing references to that renderer are invalidated, and the behaviour is undefined if it is attempted to be used.
		 * @param rh Opaque handle associated with an existing renderer.
		 */
		void destroy_renderer(tz::gl::renderer_handle rh);
		/**
		 * Retrieve the number of existing renderers.
		 * @note This includes internal renderers. Because of this, this may return a value greater than the number of renderers you have created, however it will never be lower.
		 * @return Number of existing renderers.
		 */
		std::size_t renderer_count();
		/**
		 * Retrieve a reference to the renderer associated with the given handle.
		 * @pre The renderer associated with the handle `rh` must still be existing. This means that it has been created by @ref create_renderer() but has not yet been destroyed via @ref destroy_renderer().
		 * @param rh Opaque handle associated with an existing renderer.
		 * @return Reference to the associated renderer.
		 */
		const tz::gl::renderer& get_renderer(tz::gl::renderer_handle rh) const;
		/**
		 * Retrieve a reference to the renderer associated with the given handle.
		 * @pre The renderer associated with the handle `rh` must still be existing. This means that it has been created by @ref create_renderer() but has not yet been destroyed via @ref destroy_renderer().
		 * @param rh Opaque handle associated with an existing renderer.
		 * @return Reference to the associated renderer.
		 */
		tz::gl::renderer& get_renderer(tz::gl::renderer_handle rh);

		/**
		 * Retrieve the image format that is being used by the window surface.
		 * @return Image format matching the internal window surface.
		 */
		tz::gl::image_format get_window_format() const;
	};
	#endif
}

#include "tz/gl/api/device.inl"
#endif // TOPAZ_GL_2_API_DEVICE_HPP
