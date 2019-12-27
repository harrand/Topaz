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
     * tz::gl::Objects represent arbitrary Objects stored in VRAM. In OpenGL nomenclature, it is little more than a glorified VAO.
     * 
     * tz::gl::Objects are responsible for ownership of at least zero tz::gl::Buffers. Some Buffers need no Object parent to work, but some do.
     */
    class Object
    {
    public:
        /**
         * Construct an empty Object. This object will be valid to the eyes of the graphics card drivers but has no useful information.
         */
        Object();
        /**
         * Cleans up VRAM resources used and will destroy all parent Buffers.
         */
        ~Object();
        /**
         * Bind the Object. TODO: Document this better.
         */
        void bind() const;
        /**
         * Unbind the Object. TODO: Document this better.
         */
        void unbind() const;

        /**
         * Retrieve the number of Buffers that this Object owns.
         * @return Number of child buffers.
         */
        std::size_t size() const;

        bool operator==(ObjectHandle handle) const;
        bool operator==(const Object& rhs) const;
        bool operator!=(ObjectHandle handle) const;
        bool operator!=(const Object& rhs) const;

        /**
         * Take ownership of an existing Buffer and retrieve an opaque ID handle corresponding to that Buffer.
         * 
         * Note: This handle can be passed to operator[] to retrieve a pointer to the Buffer.
         * @param buffer The buffer to take ownership of.
         * @return Handle ID of the now-owned Buffer.
         */
        std::size_t add_buffer(std::unique_ptr<tz::gl::IBuffer> buffer);
        /**
         * Create a new Buffer in-place and retrieve an opaque ID handle corresponding to the new Buffer.
         * 
         * Note: This handle can be passed to operator[] to retrieve a pointer to the Buffer.
         * @tparam Type The type specialisation to create the Buffer.
         * @tparam Args Types of arguments used to construct the Buffer.
         * @param args Values of arguments used to construct the Buffer.
         * @return Handle ID of the newly-created and owned Buffer.
         */
        template<tz::gl::BufferType Type, typename... Args>
        std::size_t emplace_buffer(Args&&... args);

        /**
         * Retrieve a pointer to an existing Buffer using its Handle ID.
         * 
         * Note: This can return nullptr if the Buffer at this index was previously erased or released.
         * Precondition: The given index must be in-range (0 <= idx <= this->size()). Otherwise this will assert and invoke UB.
         * @param idx Handle ID whose corresponding Buffer should be retrieved.
         * @return Pointer to the existing Buffer if it exists. Otherwise nullptr.
         */
        tz::gl::IBuffer* operator[](std::size_t idx);
        /**
         * Retrieve a pointer to an existing IBuffer using its Handle ID. This will return the underlying interface.
         * 
         * Note: This can return nullptr if the Buffer at this index was previously erased or released.
         * Precondition: The given index must be in-range (0 <= idx <= this->size()). Otherwise this will assert and invoke UB.
         * @param idx Handle ID whose corresponding Buffer should be retrieved.
         * @return Pointer to the existing Buffer if it exists. Otherwise nullptr.
         */
        const tz::gl::IBuffer* operator[](std::size_t idx) const;

        /**
         * Bind this Object, and then bind the child at the given index. This corresponds to the handle ID used when creating or taking ownership of the Buffer.
         * Precondition: See IBuffer::bind()
         * Precondition: The given index must be in-range (0 <= idx <= this->size()). Otherwise this will assert and invoke UB.
         * @param idx Handle ID whose corresponding Buffer should be bound.
         */
        void bind_child(std::size_t idx) const;
        /**
         * Retrieve a pointer to an existing IBuffer using its Handle ID. This will return the underlying interface.
         * 
         * Note: This can return nullptr if the Buffer at this index was previously erased or released.
         * Note: If the underlying type o the Buffer is not known, you can instead retrieve a pointer to the interface via this->operator[].
         * Precondition: The given index must be in-range (0 <= idx <= this->size()). Otherwise this will assert and invoke UB.
         * Precondition: The Buffer at the given index must have underlying type matching Type. Otherwise this will invoke UB without asserting.
         * @tparam Type Underlying type of the Buffer at the given index.
         * @param idx Handle ID whose corresponding Buffer should be retrieved.
         * @return Pointer to the existing Buffer if it exists. Otherwise nullptr.
         */
        template<tz::gl::BufferType Type>
        tz::gl::Buffer<Type>* get(std::size_t idx);
        /**
         * Retrieve a pointer to an existing IBuffer using its Handle ID. This will return the underlying interface.
         * 
         * Note: This can return nullptr if the Buffer at this index was previously erased or released.
         * Note: If the underlying type o the Buffer is not known, you can instead retrieve a pointer to the interface via this->operator[].
         * Precondition: The given index must be in-range (0 <= idx <= this->size()). Otherwise this will assert and invoke UB.
         * Precondition: The Buffer at the given index must have underlying type matching Type. Otherwise this will invoke UB without asserting.
         * @tparam Type Underlying type of the Buffer at the given index.
         * @param idx Handle ID whose corresponding Buffer should be retrieved.
         * @return Pointer to the existing Buffer if it exists. Otherwise nullptr.
         */
        template<tz::gl::BufferType Type>
        const tz::gl::Buffer<Type>* get(std::size_t idx) const;
    private:
        void verify() const;

        ObjectHandle vao;
        std::vector<std::unique_ptr<tz::gl::IBuffer>> buffers;
    };

    namespace bound
    {
        /**
         * Retrieve the currently bound VAO. This will return 0 if no VAO is bound.
         * @return Positive integer representing bound VAO if there is one. Otherwise 0.
         */
        int vao();
    }
}

#include "gl/object.inl"
#endif // TOPAZ_GL_OBJECT_HPP