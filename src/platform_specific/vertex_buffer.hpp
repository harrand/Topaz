//
// Created by Harrand on 04/04/2019.
//

#ifndef TOPAZ_VERTEX_BUFFER_HPP
#define TOPAZ_VERTEX_BUFFER_HPP

#include "core/topaz.hpp"

#ifdef TOPAZ_OPENGL
namespace tz::platform
{
    enum class OGLVertexBufferTarget : GLenum
    {
        ARRAY = GL_ARRAY_BUFFER,
        ELEMENT_ARRAY = GL_ELEMENT_ARRAY_BUFFER
    };

    enum class OGLVertexBufferFrequency
    {
        STREAM, STATIC, DYNAMIC
    };

    enum class OGLVertexBufferNature
    {
        DRAW, READ, COPY
    };

    class OGLVertexBufferUsage
    {
    public:
        OGLVertexBufferUsage(OGLVertexBufferFrequency frequency, OGLVertexBufferNature nature);
        friend class OGLVertexBuffer;
    private:
        GLenum operator()() const;

        OGLVertexBufferFrequency frequency;
        OGLVertexBufferNature nature;
    };

    class OGLVertexBuffer
    {
    public:
        OGLVertexBuffer(OGLVertexBufferTarget target);
        ~OGLVertexBuffer();
        std::size_t get_size() const;
        bool empty() const;
        void allocate_memory(std::size_t size, const OGLVertexBufferUsage& usage) const;
        template<template<typename> typename Container, typename POD>
        void insert(const Container<POD>& data, const OGLVertexBufferUsage& usage) const;
        void update(GLintptr offset, GLsizeiptr size, const void* data) const;
        template<template<typename> typename Container, typename POD>
        std::optional<Container<POD>> query_all_data() const;
        void bind() const;
        void unbind() const;
    private:
        OGLVertexBufferTarget target;
        GLuint vbo_handle;
    };

    enum class OGLVertexAttributeType : GLenum
    {
        SHORT = GL_SHORT,
        USHORT = GL_UNSIGNED_SHORT,
        INT = GL_INT,
        UINT = GL_UNSIGNED_INT,
        FLOAT = GL_FLOAT,
        DOUBLE = GL_DOUBLE
    };

    class OGLVertexAttribute
    {
    public:
        OGLVertexAttribute(GLuint attribute_id);
        void enable() const;
        void disable() const;
        template<typename T>
        void define(GLint size, GLboolean normalised, GLsizei stride) const;
        void direct_define(GLint size, const OGLVertexAttributeType& type, GLboolean normalised, GLsizei stride) const;
        void instanced_define(GLuint divisor) const;
    private:
        GLuint attribute_id;
    };

    class OGLVertexArray
    {
    public:
        OGLVertexArray();
        OGLVertexArray(const OGLVertexArray& copy);
        OGLVertexArray(OGLVertexArray&& move);
        ~OGLVertexArray();
        OGLVertexArray& operator=(OGLVertexArray rhs);
        void bind() const;
        void unbind() const;
        template<typename... Args>
        OGLVertexBuffer& emplace_vertex_buffer(Args&&... args);
        template<typename... Args>
        OGLVertexAttribute& emplace_vertex_attribute(Args&&... args);
        bool operator==(const OGLVertexArray& rhs) const;

        static void swap(OGLVertexArray& lhs, OGLVertexArray& rhs);
    private:
        GLuint vao_handle;
        std::vector<std::unique_ptr<OGLVertexBuffer>> vertex_buffers;
        std::vector<std::unique_ptr<OGLVertexAttribute>> vertex_attributes;
    };
}
#endif

#include "vertex_buffer.inl"
#endif //TOPAZ_VERTEX_BUFFER_HPP
