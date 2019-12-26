//
// Created by Harrand on 25/12/2019.
//

#ifndef TOPAZ_GL_OBJECT_HPP
#define TOPAZ_GL_OBJECT_HPP
#include "gl/buffer.hpp"
#include <vector>
#include <memory>

namespace tz::gl
{
    using ObjectHandle = GLuint;

    /**
     * TODO: Document
     */
    class Object
    {
    public:
        /**
         * TODO: Document
         */
        Object();
        /**
         * TODO: Document
         */
        ~Object();
        /**
         * TODO: Document
         */
        void bind() const;
        /**
         * TODO: Document
         */
        void unbind() const;

        /**
         * TODO: Document
         */
        std::size_t size() const;

        bool operator==(ObjectHandle handle) const;
        bool operator==(const Object& rhs) const;
        bool operator!=(ObjectHandle handle) const;
        bool operator!=(const Object& rhs) const;

        std::size_t add_buffer(std::unique_ptr<tz::gl::IBuffer> buffer);
        template<tz::gl::BufferType Type, typename... Args>
        std::size_t emplace_buffer(Args&&... args);

        tz::gl::IBuffer* operator[](std::size_t idx);
        const tz::gl::IBuffer* operator[](std::size_t idx) const;

        void bind_child(std::size_t idx) const;

        template<tz::gl::BufferType Type>
        tz::gl::Buffer<Type>* get(std::size_t idx);
        template<tz::gl::BufferType Type>
        const tz::gl::Buffer<Type>* get(std::size_t idx) const;
    private:
        void verify() const;

        ObjectHandle vao;
        std::vector<std::unique_ptr<tz::gl::IBuffer>> buffers;
    };

    namespace bound
    {
        int vao();
    }
}

#include "gl/object.inl"
#endif // TOPAZ_GL_OBJECT_HPP