#ifndef TOPAZ_GL2_API_INPUT_HPP
#define TOPAZ_GL2_API_INPUT_HPP
#include "core/handle.hpp"
#include <span>
#include <cstddef>

namespace tz::gl2
{
	namespace detail
	{
		struct InputHandleType{};
	}
	/**
	 * @ingroup tz_gl2_io
	 * Opaque handle which is used to refer to an existing @ref Input within a Renderer or Processor.
	 */
	using InputHandle = tz::Handle<detail::InputHandleType>;

	/**
	 * @ingroup tz_gl2_io
	 * Describes the manner in which an input can be read or written to, when owned by a Renderer or Processor.
	 */
	enum class InputAccess
	{
		/// - Input data is set once, and cannot be resized.
		StaticFixed,
		/// - Input data is set once, but resizeable.
		StaticVariable,
		/// - Input data is editable, but cannot be resized.
		DynamicFixed,
		/// - Input data is editable and resizeable.
		DynamicVariable
	};

	/**
	 * @ingroup tz_gl2_io
	 * Interface for a Renderer or Processor input.
	 *
	 * Inputs are blocks of data that is interpreted in an implementation-defined manner. Specifically, the shader. For this reason, the vertex data of an input is represented as a series of bytes. Concrete representations of inputs can exist, and you are encouraged to create your own if you have a fixed input format.
	 *
	 * Alternatively, there is a default concrete implementation for typical meshes in @ref MeshInput.
	 */
	class IInput
	{
	public:
		IInput() = default;
		/**
		 * Retrieve access information about this input when used in a Renderer or Processor.
		 * @return InputAccess corresponding to usage in a Renderer or Processor.
		 */
		virtual InputAccess get_access() const = 0;
		/**
		 * Retrieve a read-only view to the input's vertex data.
		 */
		virtual std::span<const std::byte> vertex_data() const = 0;
		/**
		 * Retrieve a read+write view into the input's vertex data.
		 * @pre If this input is owned by a Renderer or Processor, `get_access()` must return any of the dynamic values. Otherwise, the behaviour of a write is undefined.
		 */
		virtual std::span<std::byte> vertex_data() = 0;
		/**
		 * Retrieve a read-only view into the input's index data.
		 */
		virtual std::span<const unsigned int> index_data() const = 0;
		/**
		 * Retrieve a read+write view into the input's vertex data.
		 * @pre If this input is owned by a Renderer or Processor, `get_access()` must return any of the dynamic values. Otherwise, the behaviour of a write is undefined.
		 */
		virtual std::span<unsigned int> index_data() = 0;
	};
}

#endif // TOPAZ_GL2_API_INPUT_HPP
