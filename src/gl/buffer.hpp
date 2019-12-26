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

    class IBuffer
    {
    public:
        IBuffer();
        virtual ~IBuffer();

        virtual void bind() const = 0;
        virtual void unbind() const = 0;

        virtual std::size_t size() const = 0;
        bool empty() const;
        bool valid() const;
        
        virtual void resize(std::size_t size_bytes) = 0;
        
        virtual void* map(MappingPurpose purpose = MappingPurpose::ReadWrite) = 0;
        virtual void unmap() = 0;

        bool operator==(BufferHandle handle) const;
        bool operator!=(BufferHandle handle) const;
    protected:
        void verify() const;
        void verify_bound() const;

        BufferHandle handle;
    };

    template<BufferType T>
    class Buffer : public IBuffer
    {
    public:
        virtual void bind() const override;
        virtual void unbind() const override;
        virtual std::size_t size() const override;
        virtual void resize(std::size_t size_bytes) override;

        virtual void* map(MappingPurpose purpose = MappingPurpose::ReadWrite) override;
        virtual void unmap() override;

        //template<typename T>
        //tz::mem::UniformPool<T> map(std::size_t element_count, MappingPurpose purpose = MappingPurpose::ReadWrite);
    };

    // Various aliases...
    using VertexBuffer = Buffer<BufferType::Array>;
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