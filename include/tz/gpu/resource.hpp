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
	 * @defgroup tz_gpu_resource Resources
	 * @brief Create/edit/destroy resources (images and buffers).
	 **/

	/**
	 * @ingroup tz_gpu_resource
	 * @brief Specifies optional behaviours for a buffer.
	 **/
	enum buffer_flag
	{
		/// Buffer can be used as an index buffer by a graphics pass.
		index = 0b0001,
		/// Buffer can be used as a draw buffer by a graphics pass.
		draw = 0b0010,
		/// Buffer data will be writable directly from the CPU. Resource writes targetting buffers that are dynamic_access will be extremely fast. On some hardware, dynamic_access buffers will be slower to read/write from a shader.
		dynamic_access = 0b0100,
	};

	constexpr buffer_flag operator|(buffer_flag lhs, buffer_flag rhs)
	{
		return static_cast<buffer_flag>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}

	constexpr bool operator&(buffer_flag lhs, buffer_flag& rhs)
	{
		return static_cast<int>(lhs) & static_cast<int>(rhs);
	}

	/**
	 * @ingroup tz_gpu_resource
	 * @brief Specifies creation flags for a buffer.
	 *
	 * See @ref tz::gpu::create_buffer for usage.
	 **/
	struct buffer_info
	{
		/// What initial data shall I have? @warning This must not be empty -- zero-sized buffers are not supported.
		std::span<const std::byte> data;
		/// What name shall I have when you're looking at me in your graphics debugger of choice? If you don't specify one, I will be named "Resource 123B" (not necessarily 123)
		const char* name = "<untitled buffer resource>";
		/// Any extra optional flags to specify?
		buffer_flag flags = static_cast<buffer_flag>(0);
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
	 * @brief Specifies optional behaviours for a buffer.
	 **/
	enum image_flag
	{
		/// Image can be used as a colour target by a graphics pass.
		colour_target = 0b0001,
		/// Image can be used as a depth target by a graphics pass.
		depth_target = 0b0010,
	};

	constexpr image_flag operator|(image_flag lhs, image_flag rhs)
	{
		return static_cast<image_flag>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}

	constexpr bool operator&(image_flag lhs, image_flag& rhs)
	{
		return static_cast<int>(lhs) & static_cast<int>(rhs);
	}

	/**
	 * @ingroup tz_gpu_resource
	 * @brief Specifies creation flags for an image.
	 *
	 * See @ref tz::gpu::create_image for usage.
	 **/
	struct image_info
	{
		/// Width of the image, in pixels.
		unsigned int width;
		/// Height of the image, in pixels.
		unsigned int height;
		/// Describes the format of the image data.
		image_type type;
		/// What initial data shall I have? @note The size of the data, in bytes, should be equal to `width * height * 4` (all @ref image_type values are 32-bit aka 4 bytes per pixel). 
		std::span<const std::byte> data;
		/// What name shall I have when you're looking at me in your graphics debugger of choice? If you don't specify one, I will be named "Resource 123I" (not necessarily 123)
		const char* name = "<untitled image resource>";
		/// Any extra optional flags to specify?
		image_flag flags = static_cast<image_flag>(0);
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

	void resource_write(resource_handle res, std::span<const std::byte> new_data);
}

#endif // TOPAZ_GPU_RESOURCE_HPP