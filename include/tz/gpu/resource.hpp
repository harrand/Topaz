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
		/// Image will be automatically resized to match the dimensions of the window. In addition, @ref tz::gpu::image_info::width and @ref tz::gpu::image_info::height are ignored.
		resize_to_match_window_resource = 0b0100,
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
	 * @brief Corresponds to either the window image (as a colour target) or the system depth image (as a depth target). Note that it is invalid to use this value for anything other than @ref pass_graphics_state::colour_targets or @ref pass_graphics_state::depth_target.
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
	 * @return @ref tz::error_code::invalid_value If you attempt to delete the null resource @ref tz::nullhand.
	 * @return @ref tz::error_code::invalid_value If you attempt to delete the window resource @ref tz::gpu::window_resource.
	 * @return @ref tz::error_code::invalid_value If the resource handle provided is invalid. This usually happens due to memory corruption or an accidental double-delete.
	 * @return @ref tz::error_code::concurrent_usage If the resource is being used by at least one pass.
	 *
	 * If you know a resource uses alot of memory, or you're sure you're not going to use it anymore, you can destroy it here, causing all of its memory to be available for reuse.
	 *
	 * @note If you never destroy a resource manually, it will automatically be destroyed for you when you call @ref tz::terminate.
	 *
	 * @warning You must not delete a resource that has been registered for use by an existing pass, until you delete all such passes first.
	 * 
	 **/
	tz::error_code destroy_resource(resource_handle res);

	/**
	 * @ingroup tz_gpu
	 * @brief Represents a single index. Indices are always 32-bit unsigned integers.
	 */
	using index_t = std::uint32_t;
	/**
	 * @ingroup tz_gpu
	 * @brief Represents a single unindexed draw-call.
	 *
	 * See @ref pass_graphics_state::draw_buffer for details.
	 */
	struct draw_t
	{
		/// Number of vertices to draw.
		std::uint32_t vertex_count = 0;
		/// Number of instances to draw. If you're not doing GPU instancing, you probably want this to be 1.
		std::uint32_t instance_count = 1;
		/// Index of the first vertex to draw. Essentially offsets gl_VertexIndex. (TODO: confirm that this is true)
		std::uint32_t first_vertex = 0;
		/// Instance ID of the first instance to draw.
		std::uint32_t first_instance = 0;
	};
	/**
	 * @ingroup tz_gpu
	 * @brief Represents a single indexed draw-call.
	 * 
	 * See @ref pass_graphics_state::draw_buffer for details.
	 */
	struct draw_indexed_t
	{
		/// Number of indices (vertices) to draw.
		std::uint32_t index_count = 0;
		/// Number of instances to draw. If you're not doing GPU instancing, you probably want this to be 1.
		std::uint32_t instance_count = 1;
		/// Index of the first index to draw (within the index buffer).
		std::uint32_t first_index = 0;
		/// An offset added to each index value before indexing into the vertex buffer.
		std::int32_t vertex_offset = 0;
		/// Instance ID of the first instance to draw.
		std::uint32_t first_instance = 0;
	};

	/**
	 * @ingroup tz_gpu_resource
	 * @brief Write some new data to a resource. The thread will block until the changes are resident GPU-side.
	 * @param res Resource whose data should be changed.
	 * @param new_data Region containing new memory for the provided resource.
	 * @return @ref tz::error_code::invalid_value If the write is too large. The size of the new data + offset must be less than or equal to the resource's size.
	 *
	 * If the region of new data is smaller than the total size of the resource's underlying data, then all bytes beyond the new data region will be unchanged and keep their previous state.
	 *
	 * Regarding buffer resources:
	 * - If the buffer is dynamic_access, the cost of the transfer should be in the same ballpark as a memcpy. In all other cases, a resource write is *much* slower.
	 * Regarding image resources:
	 * - The data should be in the format of a tightly-packed array containing rows of pixels.
	 * 		- If the image was created with @ref image_type::rgba, then each pixel should be 4 bytes - one for each component (0-255).
	 * 		- If the image was created with @ref image_type::depth or @ref image_type::floats, then each pixel should be 4 bytes - a single signed 32-bit float.
	 */
	tz::error_code resource_write(resource_handle res, std::span<const std::byte> new_data, std::size_t offset = 0);
	/**
	 * @ingroup tz_gpu_resource
	 * @brief Retrieve the size of a resource's data, in bytes.
	 */
	std::size_t resource_size(resource_handle res);
	/**
	 * @ingroup tz_gpu_resource
	 * @brief Retrieve the width of an image resource.
	 *
	 * - It is valid to pass @ref tz::gpu::window_resource, in which case the width of the window image will be returned.
	 * - If you do not pass a valid image resource (e.g a buffer resource, or the null handle), then 0 is returned.
	 */
	unsigned int image_get_width(resource_handle res);
	/**
	 * @ingroup tz_gpu_resource
	 * @brief Retrieve the height of an image resource.
	 *
	 * - It is valid to pass @ref tz::gpu::window_resource, in which case the height of the window image will be returned.
	 * - If you do not pass a valid image resource (e.g a buffer resource, or the null handle), then 0 is returned.
	 */
	unsigned int image_get_height(resource_handle res);
	/**
	 * @ingroup tz_gpu_resource
	 * @brief Retrieves the current data within a resource.
	 *
	 * You aren't allowed to read from @ref tz::gpu::window_resource.
	 */
	std::span<const std::byte> resource_read(resource_handle res);
	/**
	 * @ingroup tz_gpu_resource
	 * @brief Resize a buffer resource.
	 *
	 * - If the size of the buffer increases, the new memory at the end will be of unspecified value.
	 * - Any data within the buffer that you have previously written to will be preserved.
	 */
	void buffer_resize(resource_handle bufh, std::size_t new_size_bytes);
	/**
	 * @ingroup tz_gpu_resource
	 * @brief Resize an image resource.
	 *
	 * - If the size of the image increases, the new rows/columns of the image will be of unspecified value.
	 * - Any data within the image that you have previously written to will be preserved.
	 */
	void image_resize(resource_handle imgh, unsigned int new_width, unsigned int new_height);
	/**
	 * @ingroup tz_gpu_resource
	 * @brief Write indices into a buffer resource.
	 *
	 * This is a helper function which will call @ref resource_write under-the-hood.
	 */
	tz::error_code index_buffer_write(resource_handle index_buffer, std::span<const index_t> indices);
	/**
	 * @ingroup tz_gpu_resource
	 * @brief Write draw-indirect-count + commands into a buffer resource.
	 *
	 * This is a helper function which will call @ref resource_write under-the-hood.
	 */
	tz::error_code draw_buffer_write(resource_handle draw_buffer, std::uint32_t count, std::span<const draw_t> draws);
	/**
	 * @ingroup tz_gpu_resource
	 * @brief Write draw-indirect-count + indexed commands into a buffer resource.
	 *
	 * This is a helper function which will call @ref resource_write under-the-hood.
	 */
	tz::error_code draw_buffer_indexed_write(resource_handle draw_buffer, std::uint32_t count, std::span<const draw_indexed_t> draws);
}

#endif // TOPAZ_GPU_RESOURCE_HPP
