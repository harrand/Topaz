#ifndef TOPAZ_GPU_RESOURCE_HPP
#define TOPAZ_GPU_RESOURCE_HPP
#include "tz/core/error.hpp"
#include "tz/core/handle.hpp"
#include <span>
#include <expected>

namespace tz::gpu
{
	/**
	 * @ingroup tz_gpu
	 * @defgroup tz_gpu_resource GPU Resources
	 * @brief Create/edit/destroy resources (images and buffers).
	 **/

	/**
	 * @ingroup tz_gpu_resource
	 * @brief Describes how often you are expected to access (write) to the resource's data.
	 *
	 * Resources that are static-access are highly likely to be located in GPU memory, which is the fastest to read from during GPU work, but has a very high cost to write to.
	 *
	 * Resources that are dynamic-access, however, are always writable from the CPU with minimal latency. This will mostly be much slower to read from during GPU work, although there are some exceptions to this depending on your hardware (e.g nvidia REBAR, amd SAM)
	 **/
	enum class resource_access
	{
		/// Indicates you do not expect to write to the resource's data often, and are willing to incur a large stall when you do.
		static_access,
		/// Indicates that you intend to write to the resource's data very often, perhaps even on a per-frame basis.
		dynamic_access
	};

	/**
	 * @ingroup tz_gpu_resource
	 * @brief Specifies what a buffer resource shall be used for.
	 **/
	enum class buffer_type
	{
		/// The buffer exists to store data for read/write by a shader running on the GPU.
		storage,
		/// The buffer exists purely to store index data for use when rendering a mesh. Index buffers are expected to be filled entirely with std::uint32_t.
		index,
		/// The buffer mainly contains draw commands with a count at the front, intended for use in GPU-driven rendering.
		draw_indirect
	};

	/**
	 * @ingroup tz_gpu_resource
	 * @brief Specifies creation flags for a buffer.
	 *
	 * See @ref tz::gpu::create_buffer for usage.
	 **/
	struct buffer_info
	{
		/// Do you intend to write to my buffer data very rarely (static) or very often, perhaps even on a per-frame basis (dynamic)?
		resource_access access;
		/// What kind of GPU work do you intend to use me for?
		buffer_type type;
		/// What initial data shall I have? @warning This must not be empty -- zero-sized buffers are not supported.
		std::span<const std::byte> data;
		/// What name shall I have when you're looking at me in your graphics debugger of choice? If you don't specify one, I will be named "Resource 123B" (not necessarily 123)
		const char* debug_name = "";
	};

	/**
	 * @ingroup tz_gpu_resource
	 * @brief Describes the internal format of an image's data.
	 *
	 * You are not able to choose a specific sized format.
	 **/
	enum class image_type
	{
		/// Image data is in a 32-bit RGBA unsigned normalised format.
		rgba,
		/// Image is comprised of 32-bit floats per pixel, but is only used as a depth image.
		depth,
		/// Image is comprised of 32-bit floats per pixel, but cannot be used as a depth image. Useful for normal maps.
		floats,
	};

	/**
	 * @ingroup tz_gpu_resource
	 * @brief Specifies creation flags for an image.
	 *
	 * See @ref tz::gpu::create_image for usage.
	 **/
	struct image_info
	{
		/// Do you intend to write to my buffer data very rarely (static) or very often, perhaps even on a per-frame basis (dynamic)?
		resource_access access;
		/// Width of the image, in pixels.
		unsigned int width;
		/// Height of the image, in pixels.
		unsigned int height;
		/// Describes the format of the image data.
		image_type type;
		/// What initial data shall I have? @note The size of the data, in bytes, should be equal to `width * height * 4` (all @ref image_type values are 32-bit aka 4 bytes per pixel). 
		std::span<const std::byte> data;
		/// What name shall I have when you're looking at me in your graphics debugger of choice? If you don't specify one, I will be named "Resource 123I" (not necessarily 123)
		const char* debug_name = "";
	};

	/**
	 * @ingroup tz_gpu_resource
	 * @brief Corresponds to a previously-created resource (buffer or image).
	 */
	using resource_handle = tz::handle<buffer_info>;
	/**
	 * @ingroup tz_gpu_resource
	 * @brief Corresponds to either the window image (as a colour target) or the system depth image (as a depth target).
	 */
	constexpr auto window_resource = static_cast<tz::hanval>(std::numeric_limits<std::underlying_type_t<tz::hanval>>::max() - 1);

	/**
	 * @ingroup tz_gpu_resource
	 * @brief Create a new buffer.
	 * @return On success: A @ref resource_handle corresponding to the newly-created buffer. 
	 * @return @ref tz::error_code::precondition_failure If the buffer info you provided was malformed (99% you are trying to use a zero-sized buffer which isn't supported).
	 * @return @ref tz::error_code::oom If CPU memory is exhausted whilst trying to create the buffer.
	 * @return @ref tz::error_code::voom If GPU memory is exhausted whilst trying to create the buffer.
	 * @return @ref tz::error_code::unknown_error If some other undocumented error occurs.
	 *
	 *
	 * Once you successfully create a resource, you can use the returned @ref resource_handle to create a new renderer that uses that resource.
	 *
	 * Once you're done with a resource, you can destroy it using @ref destroy_resource.
	 * @note If you never destroy a resource manually, it will automatically be destroyed for you when you call @ref tz::terminate.
	 **/
	std::expected<resource_handle, tz::error_code> create_buffer(buffer_info);
	/**
	 * @ingroup tz_gpu_resource
	 * @brief Create a new image.
	 * @return On success: A @ref resource_handle corresponding to the newly-created image. 
	 * @return @ref tz::error_code::oom If CPU memory is exhausted whilst trying to create the image.
	 * @return @ref tz::error_code::voom If GPU memory is exhausted whilst trying to create the image.
	 * @return @ref tz::error_code::unknown_error If some other undocumented error occurs.
	 *
	 *
	 * Once you successfully create a resource, you can use the returned @ref resource_handle to create a new renderer that uses that resource.
	 *
	 * Once you're done with a resource, you can destroy it using @ref destroy_resource.
	 * @note If you never destroy a resource manually, it will automatically be destroyed for you when you call @ref tz::terminate.
	 **/
	std::expected<resource_handle, tz::error_code> create_image(image_info);

	/**
	 * @ingroup tz_gpu_resource
	 * @brief Manually destroy a resource.
	 *
	 * If you know a resource uses alot of memory, or you're sure you're not going to use it anymore, you can destroy it here, causing all of its memory to be available for reuse.
	 *
	 * @note If you never destroy a resource manually, it will automatically be destroyed for you when you call @ref tz::terminate.
	 *
	 * @warning Be aware of GPU synchronisation! Ensure that you do not destroy a resource that is currently in-use by ongoing GPU work. Take care to ensure that renderers using the resource have finished their work before tearing it down.
	 * 
	 **/
	tz::error_code destroy_resource(resource_handle res);
}

#endif // TOPAZ_GPU_RESOURCE_HPP