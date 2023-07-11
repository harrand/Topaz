#ifndef TOPAZ_GL2_API_RESOURCE_HPP
#define TOPAZ_GL2_API_RESOURCE_HPP
#include "tz/core/memory/clone.hpp"
#include "tz/core/data/handle.hpp"
#include "tz/core/data/enum_field.hpp"
#include <span>
#include <memory>

namespace tz::gl
{

	/**
	 * @ingroup tz_gl2_res
	 * <a href="https://github.com/harrand/Topaz/wiki/Renderer-Resources">Resources</a> 
	 */
	enum class resource_type
	{
		/// - resource is a buffer resource, which contains arbitrary data.
		buffer,
		/// - resource is an image resource, of some format and dimensions.
		image
	};

	/**
	 * @ingroup tz_gl2_res
	 * Specifies optional flags which affect the behaviour of a resource in some way.
	 */
	enum class resource_flag
	{
		/// - Indicates that the buffer should be treated as a hardware index buffer. It will act as a bespoke non-shader-resource buffer that must store indices encoded as `unsigned int[]`. Can only be applied to buffer resources.
		index_buffer,
		/// - Indicates that the buffer should contain draw commands which will be used in a render invocation.
		draw_indirect_buffer,
		/// - Indicates that the image can be used as an image_output for another renderer. Can only be applied to image resources.
		renderer_output,
		/// - Indicates that when doing min/mag on the image, the value of the nearest texel to the texcoord is retrieved.
		image_filter_nearest,
		/// - Indicates that when doing min/mag on the image, the value of the weighted average of the nearest texels is retrieved. This is the default filter.
		image_filter_linear,
		/// - Indicates that the chosen mip will have the closest match of size to the texture pixel. @note Not yet implemented.
		image_mip_nearest,
		/// - Indicates that a mip computed from the weighted average of the next and previous mip will be chosen. @note Not yet implemented.
		image_mip_linear,
		/// - Indicates that if sampled outside of its dimensions, the colour of the sampled pixel will match that of the closest axis-aligned texel. This is the default wrap mode.
		image_wrap_clamp_edge,
		/// - Indicates that if sampled outside of its dimensions, the colour of the sampled pixel will begin repeating as if from zero.
		image_wrap_repeat,
		/// - Indicates that if sampled outside of this dimensions, the colour of the sampled pixel will begin repeating as if from zero, except each time the image will look mirrored.
		image_wrap_mirrored_repeat,

		Count
	};

	using resource_flags = tz::enum_field<resource_flag>;

	/**
	 * @ingroup tz_gl2_res
	 * Describes the manner in which a resource can be read or written to, when owned by a renderer or Processor.
	 */
	enum class resource_access
	{
		/// - resource data can only be written initially, or modified via a renderer edit.
		static_access,
		/// - resource data can be modified at anytime.
		dynamic_access,
		Count
	};

	/**
	 * @ingroup tz_gl2_res
	 * Interface for a renderer or Processor resource.
	 *
	 * Resources are subsidiary blocks of either buffer or image data which are used as assets within a shader. Shaders are used by either Renderers or Preprocessors.
	 *
	 * Two concrete implementations of resources exist at present by default. These are:
	 * - @ref buffer_resource
	 * - @ref image_resource
	 */
	class iresource : public tz::unique_cloneable<iresource>
	{
	public:
		iresource() = default;
		virtual ~iresource() = default;
		/**
		 * Retrieve the type of the resource.
		 */
		virtual resource_type get_type() const = 0;
		/**
		 * Retrieve access information about this resource when used in a renderer or Processor.
		 * @return resource_access corresponding to usage in a renderer or Processor.
		 */
		virtual resource_access get_access() const = 0;
		/**
		 * Retrieve a read-only view into the resource data.
		 */
		virtual std::span<const std::byte> data() const = 0;
		/**
		 * Retrieve a read+write view into the resource data.
		 * @pre If this resource is owned by a renderer or Processor, `get_access()` must return any of the dynamic values. Otherwise, the behaviour of a write is undefined.
		 */
		virtual std::span<std::byte> data() = 0;

		/**
		 * Retrieve a read+write view into the resource data, interpreted as an array of some type.
		 * @tparam T Type to interpret. The resource data is expressed as an array of this type.
		 */
		template<typename T> std::span<const T> data_as() const
		{
			const std::byte* byte_data = this->data().data();
			return {reinterpret_cast<const T*>(byte_data), this->data().size_bytes() / sizeof(T)};
		}

		/**
		 * Retrieve a read+write view into the resource data, interpreted as an array of some type.
		 * @tparam T Type to interpret. The resource data is expressed as an array of this type.
		 */
		template<typename T> std::span<T> data_as()
		{
			std::byte* byte_data = this->data().data();
			return {reinterpret_cast<T*>(byte_data), this->data().size_bytes() / sizeof(T)};
		}

		virtual void set_mapped_data(std::span<std::byte> resource_data) = 0;

		/**
		 * Retrieve a field containing all flags applied to this resource. If you didn't specify any flags for this resource, it will be empty.
		 */
		virtual const resource_flags& get_flags() const = 0;

		/**
		 * Display debug information about the resource.
		 */
		virtual void dbgui() = 0;
	};

	/// Opaque handle which is used to refer to an existing resource within a renderer or Processor.
	using resource_handle = tz::handle<iresource>;
}

#endif // TOPAZ_GL2_API_RESOURCE_HPP
