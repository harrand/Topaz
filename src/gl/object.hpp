//
// Created by Harrand on 25/12/2019.
//

#ifndef TOPAZ_GL_OBJECT_HPP
#define TOPAZ_GL_OBJECT_HPP
#include "gl/buffer.hpp"
#include "gl/format.hpp"
#include "gl/draw_command.hpp"
#include <vector>
#include <memory>

namespace tz::gl
{
    /**
     * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
     * Low-level interface for 3D and 2D hardware-accelerated graphics programming. Used in combination with the \ref tz_render "Topaz Rendering Library".
     * @{
     */

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
        Object(const Object& copy) = delete;
        Object(Object&& move);
        Object& operator=(const Object& rhs) = delete;
        Object& operator=(Object&& rhs);
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
         * Retrieve the number of Buffers that this Object owns, including null entries.
         * 
         * Note: This includes null entries, which are created via release or erase invocations.
         * Note: To retrieve the number of non-null owned Buffers, see this->element_size().
         * @return Number of child buffers.
         */
        std::size_t size() const;
        /**
         * Retrieve the number of Buffers that this Object owns, excluding null entries.
         * 
         * Note: This excludes null entries, which are created via release or erase invocations.
         * Note: To retrieve the number of owned Buffers (including null entries), see this->size().
         * @return Number of used child buffers.
         */
        std::size_t element_size() const;

        bool operator==(ObjectHandle handle) const;
        bool operator==(const Object& rhs) const;
        bool operator!=(ObjectHandle handle) const;
        bool operator!=(const Object& rhs) const;

        /**
         * Take ownership of an existing Buffer and retrieve an opaque ID handle corresponding to that Buffer.
         * 
         * Note: If the given Buffer is an index-buffer and is intended to be used for rendering, invoke this->add_index_buffer(buffer) instead.
         * Note: This handle can be passed to operator[] to retrieve a pointer to the Buffer.
         * @param buffer The buffer to take ownership of.
         * @return Handle ID of the now-owned Buffer.
         */
        std::size_t add_buffer(std::unique_ptr<tz::gl::IBuffer> buffer);
        /**
         * Take ownership of an existing Buffer and retrieve an opaque ID handle corresponding to that Buffer. Also, interpret the Buffer as an index-buffer.
         * 
         * Note: This should be used instead of add_buffer if the given Buffer is an index-buffer.
         * Note: This handle can be passed to operator[] to retrieve a pointer to the Buffer.
         * @param buffer The buffer to take ownership of.
         * @return Handle ID of the now-owned Buffer.
         */
        std::size_t add_index_buffer(std::unique_ptr<tz::gl::IBuffer> index_buffer);
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
         * Format a vertex attribute with the given index and standardised format specifier.
         */
        void format(std::size_t idx, tz::gl::Format fmt);
        /**
         * Format a vertex attribute with the given index and custom OpenGL format specifiers.
         */
        void format_custom(std::size_t idx, GLint size, GLenum type, GLboolean normalised, GLsizei stride, const void* ptr);
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
         * Note: If the underlying type of the Buffer is not known, you can instead retrieve a pointer to the interface via this->operator[].
         * Precondition: The given index must be in-range (0 <= idx <= this->size()). Otherwise this will assert and invoke UB.
         * Precondition: The Buffer at the given index must have underlying type matching Type. Otherwise this will invoke UB without asserting.
         * @tparam Type Underlying type of the Buffer at the given index.
         * @param idx Handle ID whose corresponding Buffer should be retrieved.
         * @return Pointer to the existing Buffer if it exists. Otherwise nullptr.
         */
        template<tz::gl::BufferType Type>
        const tz::gl::Buffer<Type>* get(std::size_t idx) const;
        /**
         * Erase the Buffer at the given index, and construct a replacement Buffer in-place at that same index.
         * 
         * Note: This will destroy the previous entry. You can use this->swap(...) to extract the previous entry.
         * Note: It is safe to invoke this if the index is in range but the element at the index is null (Such as from a previous erasure).
         * Precondition: The given index must be in-range (0 <= idx <= this->size()). Otherwise this will assert and invoke UB.
         * @tparam Type Underlying type fo the Buffer to be constructed at the given index.
         * @tparam Args Types of the arguments for the new Buffer's construction.
         * @param idx Handle ID at which the Buffer should be set.
         * @param Args Values of the arguments for the new Buffer's construction.
         */
        template<tz::gl::BufferType Type, typename... Args>
        void set(std::size_t idx, Args&&... args);
        /**
         * Erase the Buffer at the given index.
         * 
         * Note: The index will not be re-used in subsequent emplace_buffer or add_buffer invocations. The only way to re-use this index is to invoke this->set(...) at this index.
         * Precondition: The given index must be in-range (0 <= idx <= this->size()). Otherwise this will assert and invoke UB.
         * @param idx The index at which the Buffer should be erased.
         */
        void erase(std::size_t idx);
        /**
         * Relinquish ownership of the Buffer at the given index, yielding it to calling code. This is essentially a retrieve-and-erase. The result can trivially be added to another Object.
         * 
         * Note: After releasing at the index, expect the Buffer at the index to be nullptr.
         * Note: If the result of this invocation is discarded, the behaviour is identical to that of erasure at this index.
         * Note: The result of this method has important RAII semantics. If you allow it to go out of scope, it will destroy the Buffer. Either keep ahold of it sensibly or pass it into a new Object ASAP.
         * Precondition: The given index must be in-range (0 <= idx <= this->size()). Otherwise this will assert and invoke UB.
         * @param idx Index at which the existing Buffer should be retrieved.
         * @return Smart-pointer to the resultant Buffer.
         */
        std::unique_ptr<tz::gl::IBuffer> release(std::size_t idx);
        /**
         * Invoke a render invocation using the given index-buffer.
         * 
         * Note: All indices will be used in the render-invocation.
         * Precondition: ibo_id must correspond to an existing and valid index-buffer within this object. Otherwise, this will assert and invoke UB.
         * @param ibo_id ID Handle corresponding to an existing index-buffer within this object.
         */
        void render(std::size_t ibo_id) const;
        /**
         * Invoke a multi-render invocation using MDI via the given index-buffer.
         * 
         * Note: The sequences of indicies specified by the command-list is used in the render-invocation.
         * Note: This will early-out in the case that the command-list is empty.
         * Precondition: ibo_id must correspond to an existing and valid index-buffer within this object. Otherwise, this will assert and invoke UB.
         * Precondition: cmd_list must contain valid values and offsets for the index-buffer corresponding to the buffer at element ibo_id. Otherwise, this will assert and invoke UB.
         * @param ibo_id ID Handle corresponding to an existing idnex-buffer within this object.
         * @param cmd_list List of glDrawElementsInstancedBaseInstanceBaseVertex commands.
         */
        void multi_render(std::size_t ibo_id, tz::gl::MDIDrawCommandList cmd_list) const;
    private:
        void verify() const;
        void verify_bound() const;
        tz::gl::IBuffer* bound_index_buffer();
        void set_draw_data(const tz::gl::MDIDrawCommandList& cmd_list) const;

        ObjectHandle vao;
        std::vector<std::unique_ptr<tz::gl::IBuffer>> buffers;
        mutable tz::gl::DIBO draw_buffer;
        std::vector<std::size_t> index_buffer_ids;
        std::size_t format_count;
    };

    namespace bound
    {
        /**
         * Retrieve the currently bound VAO. This will return 0 if no VAO is bound.
         * @return Positive integer representing bound VAO if there is one. Otherwise 0.
         */
        int vao();
    }
    /**
     * @}
     */
}

#include "gl/object.inl"
#endif // TOPAZ_GL_OBJECT_HPP