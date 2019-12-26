//
// Created by Harrand on 25/12/2019.
//

#ifndef TOPAZ_GL_BUFFER_HPP
#define TOPAZ_GL_BUFFER_HPP
#include "glad/glad.h"
#include "memory/pool.hpp"

namespace tz::gl
{
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
         * Precondition: Requires the buffer to be valid and bound.
         * @return Size of the buffer, in bytes.
         */
        virtual std::size_t size() const = 0;
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
         * Precondition: Requires the buffer to be valid and bound.
         * @return True if the buffer is terminal, otherwise false.
         */
        virtual bool is_terminal() const = 0;
        /**
         * Query as to whether the buffer is 'valid' or not. A valid buffer is a buffer that has been bound at least once.
         * 
         * Note: This must be done before most buffer operations.
         * @return True if the buffer has been bound at least once, otherwise false.
         */
        bool valid() const;
        /**
         * Attempt to change the size of the buffer.
         * 
         * Precondition: Requires the buffer to be valid, bound and unmapped.
         * Note: This will not preserve any of the data within the buffer. You should do that yourself.
         * @param size_bytes Desired new size of the buffer, in bytes.
         */
        virtual void resize(std::size_t size_bytes) = 0;
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
        virtual void terminal_resize(std::size_t size_bytes) = 0;
        /**
         * Make the Buffer terminal without affecting its size.
         * 
         * Precondition: Requires the buffer to be valid, bound, non-terminal and unmapped.
         * Note: This will preserve the data within the buffer.
         */
        virtual void make_terminal() = 0;
        /**
         * Map the buffer, providing a contiguous data block that can be used from the calling code.
         * 
         * Precondition: Requires the buffer to be valid, bound and unmapped.
         * Note: Attempting to map a buffer twice will assert and invoke UB. Query IBuffer::is_mapped() to ensure that this is false before mapping.
         * @param purpose Describes what the desired use for the data is. This is an optimisation measure. If you don't intend to edit the data, providing MappingPurpose::ReadOnly will be a performance boon. The default purpose allows reading + writing.
         * @return Memory Block containing arbitrary data. The properties of this data are not guaranteed to be consistent with that of normal RAM. For example, this might be order of magnitudes slower than normal RAM.
         */
        virtual tz::mem::Block map(MappingPurpose purpose = MappingPurpose::ReadWrite) = 0;
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
        virtual void unmap() = 0;
        /**
         * Query as to whether the buffer is currently mapped or not.
         * 
         * Precondition: Requires the buffer to be both valid and bound.
         * @return True if the buffer is mapped, otherwise false.
         */
        virtual bool is_mapped() const = 0;

        bool operator==(BufferHandle handle) const;
        bool operator!=(BufferHandle handle) const;
    protected:
        /// Asserts that the underlying handle is initialised.
        void verify() const;
        /// Asserts that this buffer is bound.
        void verify_bound() const;
        /// Asserts that this buffer is non-terminal.
        void verify_nonterminal() const;

        BufferHandle handle;
    };

    template<BufferType T>
    class Buffer : public IBuffer
    {
    public:
        virtual void bind() const override;
        virtual void unbind() const override;
        virtual std::size_t size() const override;
        virtual bool is_terminal() const override;
        virtual void resize(std::size_t size_bytes) override;
        virtual void terminal_resize(std::size_t size_bytes) override;
        virtual void make_terminal() override;

        virtual tz::mem::Block map(MappingPurpose purpose = MappingPurpose::ReadWrite) override;
        virtual void unmap() override;
        virtual bool is_mapped() const override;
    };

    // Various aliases...
    using VertexBuffer = Buffer<BufferType::Array>;
    using VBO = VertexBuffer;
    using UniformBuffer = Buffer<BufferType::UniformStorage>;
    using UBO = UniformBuffer;
    using ShaderStorageBuffer = Buffer<BufferType::ShaderStorage>;
    using SSBO = ShaderStorageBuffer;

    namespace bound
    {
        int vertex_buffer();
    }
}

#include "gl/buffer.inl"
#endif // TOPAZ_GL_BUFFER_HPP