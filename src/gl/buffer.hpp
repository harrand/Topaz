//
// Created by Harrand on 25/12/2019.
//

#ifndef TOPAZ_GL_BUFFER_HPP
#define TOPAZ_GL_BUFFER_HPP
#include "glad/glad.h"
#include "memory/pool.hpp"

namespace tz::gl
{

    /**
     * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
     * @{
     */
    enum class BufferType : GLenum
	{
		Array = GL_ARRAY_BUFFER,
		AtomicCounter = GL_ATOMIC_COUNTER_BUFFER,
		CopySource = GL_COPY_READ_BUFFER,
		CopyDestination = GL_COPY_WRITE_BUFFER,
		IndirectComputeDispatchCommand = GL_DISPATCH_INDIRECT_BUFFER,
		IndirectCommandArgument = GL_DRAW_INDIRECT_BUFFER,
		Index = GL_ELEMENT_ARRAY_BUFFER,
		PixelReadTarget = GL_PIXEL_PACK_BUFFER,
		TextureDataSource = GL_PIXEL_UNPACK_BUFFER,
		QueryResult = GL_QUERY_BUFFER,
		ShaderStorage = GL_SHADER_STORAGE_BUFFER,
		TextureData = GL_TEXTURE_BUFFER,
		TransformFeedback = GL_TRANSFORM_FEEDBACK_BUFFER,
		UniformStorage = GL_UNIFORM_BUFFER,
	};

    enum class MappingPurpose : GLenum
    {
        ReadOnly = GL_READ_ONLY,
        WriteOnly = GL_WRITE_ONLY,
        ReadWrite = GL_READ_WRITE
    };

    using BufferHandle = GLuint;

    /**
     * tz::gl Buffer Interface
     * Buffers are the means through which you can read and manipulate data in VRAM. Some Buffers require a parent tz::gl::Object to function properly -- Some do not.
     * 
     * There are many different types of buffers. This class only encompasses behaviour which is shared by all buffer types. All buffers can be bound, unbound, mapped, unmapped.
     * Note: This is not strictly true -- The concept of buffer size and resizing is only available to a few buffer types. This should be refactored out.
     */
    class IBuffer
    {
    public:
        /**
         * Construct an empty Buffer.
         */
        IBuffer();
        /**
         * Destroy the buffer and all data that might exist alongside it.
         */
        virtual ~IBuffer();

        /**
         * Bind the buffer. This must be done before most buffer operations.
         */
        virtual void bind() const = 0;
        /**
         * Unbind the buffer.
         */
        virtual void unbind() const = 0;
        /**
         * Retrieve the size of the underlying data associated with the buffer, in bytes.
         * Note: This doesn't really belong in the interface. This will fail unless the BufferType is one of the following:
         * - BufferType::Array
         * - BufferType::Index
         * Precondition: Requires the buffer to be valid.
         * @return Size of the buffer, in bytes.
         */
        std::size_t size() const;
        /**
         * Query as to whether the buffer is empty or not. Buffers are empty if their size is 0 bytes.
         * 
         * Precondition: Identical to that of IBuffer::size().
         * @return True if the buffer is empty, otherwise false.
         */
        bool empty() const;
        /**
         * Query as to whether the buffer is terminal or not. Buffers are terminal if their underlying storage is immutable and thus cannot be resized.
         * Note: Once a buffer is terminal, it will remain terminal for the remainder of its lifetime. This means that any Buffer can become terminal, but terminal Buffers cannot become non-terminal.
         * Precondition: Requires the buffer to be valid.
         * @return True if the buffer is terminal, otherwise false.
         */
        bool is_terminal() const;
        /**
         * Query as to whether the buffer is 'valid' or not. A valid buffer is a buffer that has been bound at least once.
         * Note: Buffers will automatically bind and unbind themselves upon construction. Thus, Buffers should always be invalid.
         * 
         * Note: This must be done before most buffer operations.
         * @return True if the buffer has been bound at least once, otherwise false.
         */
        bool valid() const;
        /**
         * Attempt to change the size of the buffer.
         * 
         * Precondition: Requires the buffer to be valid, nonterminal and unmapped.
         * Note: This will not preserve any of the data within the buffer. You should use safe_resize for that.
         * @param size_bytes Desired new size of the buffer, in bytes.
         */
        void resize(std::size_t size_bytes);
        /**
         * Attempt to change the size of the buffer, whilst preserving all data within the buffer in the process.
         * 
         * Note: You can assume that this will early-out if size_bytes is equal to this->size().
         * Precondition: Requires the buffer to be valid, nonterminal and unmapped. Note that it is not an error to call safe_resize on a terminal buffer if the size parameter is equal to this->size(). This behaviour is not shared by this->resize().
         */
        void safe_resize(std::size_t size_bytes);
        /**
         * Retrieve a subset of the data-store.
         * 
         * Precondition: Requires the buffer to be valid. If the buffer is non-terminal, then it must also be unmapped.
         * Precondition: offset + size_bytes must be less than this->size(). Otherwise, this will assert and invoke UB.
         * Precondition: The memory block pointed to by input_data must have size greater than or equal to size_bytes. Otherwise, this will invoke UB without asserting.
         * @param offset Offset from the beginning of the data-store, in bytes.
         * @param size_bytes Size of the data-store to query, in bytes.
         * @param input_data Pointer to pre-allocated memory.
         */
        void retrieve(std::size_t offset, std::size_t size_bytes, void* input_data) const;
        /**
         * Retrieve the entirety of the data-store.
         * 
         * Precondition: Requires the buffer to be valid. If the buffer is non-terminal, then it must also be unmapped.
         * Precondition: The memory block pointed to by input_data must have size greater than or equal to this->size(). Otherwise, this will invoke UB without asserting.
         * @param offset Offset from the beginning of the data-store, in bytes.
         * @param size_bytes Size of the data-store to query, in bytes.
         * @param input_data Pointer to pre-allocated memory.
         */
        void retrieve_all(void* input_data) const;
        /**
         * Send a memory block to the data-store at the given offset.
         *
         * Note: The offset is for the destination buffer, not this source block. This will always send the entire block.
         * Note: If the block only populates a part of the data-store, then the remainder of the data is unchanged. This means that you can freely use this to only update a part of the data-store.
         * Precondition: Requires the buffer to be valid and bound. If the buffer is non-terminal, then it must also be unmapped.
         * Precondition: The given memory block must have a size less than or equal to (this->size() - offset). Otherwise, this will assert and only send the portion of the data that fits.
         * @param offset Offset from the beginning of the data store to send data to, in bytes.
         * @param output_block Block of memory to copy to the data-store at the given offset.
         */
        void send(std::size_t offset, tz::mem::Block output_block);
        /**
         * Send arbitrary data to the data-store.
         *
         * Note: This will keep copying data from the contiguous block pointed to by output_data until the data-store is completely full. In other words, the data pointed to by output_data should have size equal to this->size()
         * Precondition: Requires the buffer to be valid and bound. If the buffer is non-terminal, then it must also be unmapped.
         * Precondition: Requires output_data to point to allocated memory of size less than this->size(). Otherwise, this will invoke UB without asserting.
         * @param output_data Pointer to contiguous data used to re-fill the data-store.
         */
        void send(const void* output_data);
        /**
         * Send a range of data to the data-store.
         * 
         * Note: If the range only populates a part of the data-store, then the remainder of the data is unchanged. This means that you can freely use this to only update a part of the data-store.
         * Precondition: Requires the buffer to be valid and bound. If the buffer is non-terminal, then it must also be unmapped.
         * Precondition: Requires the distance between begin and end to be less than this->size(). Otherwise, this will assert and only send the data that fits.
         * @tparam Iter Iterator type. Usage will fail to compile if Iter does not contain an increase operator (Iter::operator++) and the typedef Iter::value_type.
         * @param begin Beginning of the range to copy to the data-store.
         * @param end End of the range to copy to the data-store.
         */
        template<typename Iter>
        void send_range(Iter begin, Iter end);
        /**
         * Change the size of the Buffer and make it terminal.
         * 
         * Terminal Buffers are fixed-size unlike normal Buffers but have additional features which may be desireable.
         * - Terminal Buffers can remain mapped concurrent with render invocations. There is no requirement to unmap terminal buffers before rendering.
         * - Mappings of Terminal Buffers will update VRAM-side as soon as possible, as opposed to updates only happening upon unmapping.
         * - In OpenGL nomenclature, Terminal Buffers are similar to Persistent Mapped Buffers (PMB).
         * 
         * Precondition: Requires the buffer to be valid, bound, non-terminal and unmapped.
         * Note: This will not preserve any of the data within the buffer. You should do that yourself.
         * @param size_bytes Desired new size of the buffer, in bytes.
         */
        void terminal_resize(std::size_t size_bytes);
        /**
         * Make the Buffer terminal without affecting its size.
         * 
         * Precondition: Requires the buffer to be valid, bound, non-terminal and unmapped.
         * Note: This will preserve the data within the buffer.
         */
        void make_terminal();
        /**
         * Map the buffer, providing a contiguous data block that can be used from the calling code.
         * 
         * Precondition: Requires the buffer to be valid and unmapped.
         * Note: Attempting to map a buffer twice will assert and invoke UB. Query IBuffer::is_mapped() to ensure that this is false before mapping.
         * @param purpose Describes what the desired use for the data is. This is an optimisation measure. If you don't intend to edit the data, providing MappingPurpose::ReadOnly will be a performance boon. The default purpose allows reading + writing.
         * @return Memory Block containing arbitrary data. The properties of this data are not guaranteed to be consistent with that of normal RAM. For example, this might be order of magnitudes slower than normal RAM.
         */
        tz::mem::Block map(MappingPurpose purpose = MappingPurpose::ReadWrite);
        /**
         * Map the buffer, providing a memory pool to be used as an array of Ts.
         * 
         * Precondition: Requires the buffer to be valid, bound and unmapped.
         * Note: Attempting to map a buffer twice will assert and invoke UB. Query IBuffer::is_mapped() to ensure that this is false before mapping.
         * @param purpose Describes what the desired use for the data is. This is an optimisation measure. If you don't intend to edit the data, providing MappingPurpose::ReadOnly will be a performance boon. The default purpose allows reading + writing.
         * @return Uniform Memory Pool containing uninitialised data. Even if the memory does contain valid Ts, the pool will not recognise them unless they're created via the pool.
         */
        template<typename T>
        tz::mem::UniformPool<T> map_pool(MappingPurpose purpose = MappingPurpose::ReadWrite);
        /**
         * Unmap the buffer, saving any edits to previously mapped data and sending it back to VRAM.
         * 
         * Precondition: Requires the buffer to be valid, bound and mapped.
         * Note: This will also work for persistently-mapped-buffers (PMBs).
         */
        void unmap();
        /**
         * Query as to whether the buffer is currently mapped or not.
         * 
         * Precondition: Requires the buffer to be both valid and bound.
         * @return True if the buffer is mapped, otherwise false.
         */
        bool is_mapped() const;

        bool operator==(BufferHandle handle) const;
        bool operator!=(BufferHandle handle) const;
    protected:
        /// Asserts that the underlying handle is initialised.
        void verify() const;
        /// Asserts that this buffer is non-terminal.
        void verify_nonterminal() const;

        BufferHandle handle;
    };

    /**
     * @}
     */

    template<BufferType T>
    class Buffer : public IBuffer
    {
    public:
        Buffer();
        virtual void bind() const override;
        virtual void unbind() const override;
    };

    template<>
    class Buffer<BufferType::ShaderStorage> : public IBuffer
    {
    public:
        Buffer(std::size_t layout_qualifier_id);

        virtual void bind() const override;
        virtual void unbind() const override;

        // Special SSBO stuff:
        std::size_t get_binding_id() const;
    protected:
        std::size_t layout_qualifier_id;
    };

    template<>
    class Buffer<BufferType::UniformStorage> : public IBuffer
    {
    public:
        Buffer(std::size_t layout_qualifier_id);

        virtual void bind() const override;
        virtual void unbind() const override;

        // Special UBO stuff:
        std::size_t get_binding_id() const;
    protected:
        std::size_t layout_qualifier_id;
    };

    /**
     * \addtogroup tz_gl
     * @{
     */
    using VertexBuffer = Buffer<BufferType::Array>;
    using VBO = VertexBuffer;
    using UniformBuffer = Buffer<BufferType::UniformStorage>;
    using UBO = UniformBuffer;
    using ShaderStorageBuffer = Buffer<BufferType::ShaderStorage>;
    using SSBO = ShaderStorageBuffer;
    using IndexBuffer = Buffer<BufferType::Index>;
    using IBO = IndexBuffer;
    /**
     * @}
     */

    namespace bound
    {
        int vertex_buffer();
        int index_buffer();
        int shader_storage_buffer();
        int buffer(BufferType type);
    }
}

#include "gl/buffer.inl"
#endif // TOPAZ_GL_BUFFER_HPP