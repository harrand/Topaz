#ifndef TOPAZ_GL_RESOURCE_WRITER_HPP
#define TOPAZ_GL_RESOURCE_WRITER_HPP
#include "memory/block.hpp"
#include "core/matrix.hpp"
#include "core/vector.hpp"

namespace tz::gl
{
    /**
	 * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
	 * @{
	 */

    /**
     * Writes data to an existing block of memory. This is likely to be some mapped tz::gl::IBuffer memory.
     * This is used to copy objects into a block of memory one-at-a-time in an ordered and contiguous manner.
     * 
     * Example:
     * Iterate through each object in the scene, using this writer to write some object data into a buffer.
     * Afterwards, the buffer can be given to a tz::render::Device as a resource-buffer and used to render in one go.
     * @tparam T Type of object to write.
     */
    template<typename T>
    class BasicResourceWriter
    {
    public:
        /**
         * Construct a writer which will work on the given memory block.
         * Note: This does not dance around the type-system. If T is POD, a new T will be constructed in-place using the conversion T(T). This is a copy by default but can be easily optimised away.
         * @param data Memory block to write objects into.
         */
        BasicResourceWriter(tz::mem::Block data);
        /**
         * Write the given object into the memory block. If this is the first write, it will be written to the very beginning of the block. Otherwise, it will be written directly after the last write.
         * Precondition: The memory block is valid and pre-allocated. Otherwise, this will invoke UB without asserting.
         * Note: If the memory block is not large enough to hold another object, the write will be aborted and false is returned.
         * @param element Object to write into the memory block.
         * @return True if write was successful, otherwise false.
         */
        bool write(T element);
        /**
         * Reset the write-count of the writer, causing the next write to happen at the beginning of the memory pool once again.
         */
        void reset();
    private:
        tz::mem::Block data;
        std::size_t write_count;
    };

    /**
     * Writes object transform data into a given memory block, one MVP-matrix at a time.
     * Generates MVP matrices on-the-fly and writes them into the memory block as a BasicResourceWriter<tz::Mat4> would.
     */
    class TransformResourceWriter
    {
    public:
        /**
         * See BasicResourceWriter<tz::Mat4>::BasicResourceWriter(tz::mem::Block).
         */
        TransformResourceWriter(tz::mem::Block data);
        /**
         * Uses the parameters to create an MVP-matrix on the fly, and then writes it into the memory block.
         * See BasicResourceWriter<tz::Mat4>::write(tz::Mat4) for more information.
         */
        bool write(tz::Vec3 pos, tz::Vec3 rot, tz::Vec3 scale, tz::Vec3 cam_pos, tz::Vec3 cam_rot, float fov, float aspect, float near, float far);
        /**
         * See BasicResourceWriter<tz::Mat4>::reset().
         */
        void reset();
    private:
        BasicResourceWriter<tz::Mat4> mat_writer;
    };
    
    /**
     * @}
     */
}

#include "gl/resource_writer.inl"
#endif // TOPAZ_GL_RESOURCE_WRITER_HPP