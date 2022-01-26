#ifndef TOPAZ_GL2_API_RESOURCE_HPP
#define TOPAZ_GL2_API_RESOURCE_HPP
#include "core/handle.hpp"
#include <span>
#include <memory>

namespace tz::gl2
{

	/**
	 * @ingroup tz_gl2_res
	 * Specifies the type of the resource; which is how a Renderer or Processor will interpret the usage of the resource within a shader.
	 */
	enum class ResourceType
	{
		/// - Resource is a buffer, which contains arbitrary data.
		Buffer,
		/// - Resource is an image of some format and dimensions.
		Image
	};

	/**
	 * @ingroup tz_gl2_res
	 * Describes the manner in which a resource can be read or written to, when owned by a Renderer or Processor.
	 */
	enum class ResourceAccess
	{
		/// - Resource data is written once initially, and cannot be resized.
		StaticFixed,
		/// - Resource data is written once initially, but resizeable.
		StaticVariable,
		/// - Resource data is always writable, but cannot be resized.
		DynamicFixed,
		/// - Resource data is always writeable and resizeable.
		DynamicVariable
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

		virtual std::unique_ptr<IResource> unique_clone() const = 0;
	};

	/// Opaque handle which is used to refer to an existing Resource within a Renderer or Processor.
	using ResourceHandle = tz::Handle<IResource>;
}

#endif // TOPAZ_GL2_API_RESOURCE_HPP
