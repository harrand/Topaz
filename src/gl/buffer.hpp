#ifndef TOPAZ_GL_BUFFER_HPP
#define TOPAZ_GL_BUFFER_HPP
#include <span>
#include <cstdint>
#include <vector>

namespace tz::gl
{
    struct BufferData
    {
        template<typename T>
        static BufferData from_array(std::span<const T> data);

        template<typename T>
        static BufferData from_value(const T& data);
        std::vector<std::byte> data;
    };
}
#include "gl/buffer.inl"
#endif // TOPAZ_GL_BUFFER_HPP