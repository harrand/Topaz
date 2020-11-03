#ifndef TOPAZ_RENDER_ELEMENT_WRITER_HPP
#define TOPAZ_RENDER_ELEMENT_WRITER_HPP
#include "core/debug/assert.hpp"

namespace tz::render
{
    template<class Element, class Writer>
    class ElementWriter
    {
    public:
        static void write([[maybe_unused]] Writer& writer, [[maybe_unused]] const Element& element)
        {
            writer.write(element);
        }
    };
}

#endif // TOPAZ_RENDER_ELEMENT_WRITER_HPP