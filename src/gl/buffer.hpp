#ifndef TOPAZ_GL_BUFFER_HPP
#define TOPAZ_GL_BUFFER_HPP
#include <span>
#include <cstdint>
#include <vector>

namespace tz::gl
{
	struct BufferData
	{
		/**
		 * @brief Retrieve buffer data from an array.
		 * 
		 * @tparam T Array value type.
		 * @param data View into the array to copy from.
		 * @return BufferData Containing the buffer data.
		 */
		template<typename T>
		static BufferData from_array(std::span<const T> data);

		/**
		 * @brief Retrieve buffer data from a single value.
		 * 
		 * @tparam T Value type.
		 * @param data Value to represent the buffer data.
		 * @return BufferData Containing the buffer data.
		 */
		template<typename T>
		static BufferData from_value(const T& data);
		std::vector<std::byte> data;
	};
}
#include "gl/buffer.inl"
#endif // TOPAZ_GL_BUFFER_HPP