#ifndef TOPAZ_RENDER_ELEMENT_WRITER_HPP
#define TOPAZ_RENDER_ELEMENT_WRITER_HPP
#include "core/debug/assert.hpp"

namespace tz::render
{
    /**
	 * \addtogroup tz_render Topaz Rendering Library (tz::render)
	 * High-level interface for 3D and 2D hardware-accelerated graphics programming. Used in combination with the \ref tz_gl "Topaz Graphics Library".
	 * @{
	 */
    /**
     * Helper class responsible for writing element data into a known resource block.
     * @tparam Element type to be written by the Writer.
     * @tparam Writer type that writes element data. The resource block should already be known by the writer.
     */
    template<class Element, class Writer>
    class ElementWriter
    {
    public:
        /**
         * Default ElementWriter implementation. Assumes element can be passed directly into writer without additional processing.
         * If this is not the case, provide a specialisation for tz::render::ElementWriter<E, W> for your resource/writer type combination.
         * @param writer Writer object ready to write element data.
         * @param element Element data to write into resource block.
         */
        static void write([[maybe_unused]] Writer& writer, [[maybe_unused]] const Element& element)
        {
            writer.write(element);
        }
    };
    /**
     * @}
     */
}

#endif // TOPAZ_RENDER_ELEMENT_WRITER_HPP