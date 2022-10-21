#ifndef TOPAZ_GL2_API_RESOURCE_HPP
#define TOPAZ_GL2_API_RESOURCE_HPP
#include "tz/core/handle.hpp"
#include "tz/core/containers/enum_field.hpp"
#include <span>
#include <memory>

namespace tz::gl
{

	/**
	 * @ingroup tz_gl2_res
	 * Specifies the type of the resource; which is how a Renderer or Processor will interpret the usage of the resource within a shader.
	 */
	enum class ResourceType
	{
		/// - Resource is a buffer resource, which contains arbitrary data.
		Buffer,
		/// - Resource is an image resource, of some format and dimensions.
		Image
	};

	/**
	 * @ingroup tz_gl2_res
	 * Specifies optional flags which affect the behaviour of a resource in some way.
	 */
	enum class ResourceFlag
	{
		/// - Indicates that the buffer should be treated as a hardware index buffer. It will act as a bespoke non-shader-resource buffer that must store indices encoded as `unsigned int[]`. Can only be applied to buffer resources.
		IndexBuffer,
		/// - Indicates that the image can be used as an ImageOutput for another renderer. Can only be applied to image resources.
		RendererOutput,
		/// - Indicates that when doing min/mag on the image, the value of the nearest texel to the texcoord is retrieved.
		ImageFilterNearest,
		/// - Indicates that when doing min/mag on the image, the value of the weighted average of the nearest texels is retrieved. This is the default filter.
		ImageFilterLinear,
		/// - Indicates that the chosen mip will have the closest match of size to the texture pixel. @note Not yet implemented.
		ImageMipNearest,
		/// - Indicates that a mip computed from the weighted average of the next and previous mip will be chosen. @note Not yet implemented.
		ImageMipLinear,
		/// - Indicates that if sampled outside of its dimensions, the colour of the sampled pixel will match that of the closest axis-aligned texel. This is the default wrap mode.
		ImageWrapClampEdge,
		/// - Indicates that if sampled outside of its dimensions, the colour of the sampled pixel will begin repeating as if from zero.
		ImageWrapRepeat,
		/// - Indicates that if sampled outside of this dimensions, the colour of the sampled pixel will begin repeating as if from zero, except each time the image will look mirrored.
		ImageWrapMirroredRepeat,

		Count
	};

	using ResourceFlags = tz::EnumField<ResourceFlag>;

	/**
	 * @ingroup tz_gl2_res
	 * Describes the manner in which a resource can be read or written to, when owned by a Renderer or Processor.
	 */
	enum class ResourceAccess
	{
		/// - Resource data is written once initially, and cannot be resized.
		StaticFixed,
		/// - Resource data is always writable, but cannot be resized.
		DynamicFixed,
		/// - Resource data is always writeable and resizeable.
		DynamicVariable,
		Count
	};

	/**
	 * @ingroup tz_gl2_res
	 * Interface for a Renderer or Processor resource.
	 *
	 * Resources are subsidiary blocks of either buffer or image data which are used as assets within a shader. Shaders are used by either Renderers or Preprocessors.
	 *
	 * Two concrete implementations of resources exist at present by default. These are:
	 * - @ref BufferResource
	 * - @ref ImageResource
	 */
	class IResource
	{
	public:
		IResource() = default;
		virtual ~IResource() = default;
		/**
		 * Retrieve the type of the resource.
		 */
		virtual ResourceType get_type() const = 0;
		/**
		 * Retrieve access information about this resource when used in a Renderer or Processor.
		 * @return ResourceAccess corresponding to usage in a Renderer or Processor.
		 */
		virtual ResourceAccess get_access() const = 0;
		/**
		 * Retrieve a read-only view into the resource data.
		 */
		virtual std::span<const std::byte> data() const = 0;
		/**
		 * Retrieve a read+write view into the resource data.
		 * @pre If this resource is owned by a Renderer or Processor, `get_access()` must return any of the dynamic values. Otherwise, the behaviour of a write is undefined.
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

		virtual std::unique_ptr<IResource> unique_clone() const = 0;

		virtual void set_mapped_data(std::span<std::byte> resource_data) = 0;

		/**
		 * Retrieve a field containing all flags applied to this resource. If you didn't specify any flags for this resource, it will be empty.
		 */
		virtual const ResourceFlags& get_flags() const = 0;

		/**
		 * Display debug information about the resource.
		 */
		virtual void dbgui() = 0;
	};

	/// Opaque handle which is used to refer to an existing Resource within a Renderer or Processor.
	using ResourceHandle = tz::Handle<IResource>;
}

#endif // TOPAZ_GL2_API_RESOURCE_HPP
