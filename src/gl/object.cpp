#include "gl/object.hpp"
#include "core/debug/assert.hpp"
#include <algorithm>

namespace tz::gl
{
    Object::Object(): vao(0), buffers(), index_buffer_ids(), format_count(0)
    {
        glGenVertexArrays(1, &this->vao);
    }

    Object::Object(Object&& move): vao(move.vao), buffers(std::move(move.buffers)), index_buffer_ids(move.index_buffer_ids), format_count(move.format_count)
    {
        move.vao = 0;
    }

    Object& Object::operator=(Object&& rhs)
    {
        std::swap(this->vao, rhs.vao);
        std::swap(this->buffers, rhs.buffers);
        std::swap(this->index_buffer_ids, rhs.index_buffer_ids);
        std::swap(this->format_count, rhs.format_count);
    }

    Object::~Object()
    {
        glDeleteVertexArrays(1, &this->vao);
    }

    void Object::bind() const
    {
        glBindVertexArray(this->vao);
    }

    void Object::unbind() const
    {
        glBindVertexArray(0);
    }

    std::size_t Object::size() const
    {
        return this->buffers.size();
    }

    std::size_t Object::element_size() const
    {
        std::size_t sz = 0;
        for(const auto& child_ptr : this->buffers)
            if(child_ptr != nullptr)
                sz++;
        return sz;
    }

    bool Object::operator==(ObjectHandle handle) const
    {
        return this->vao == handle;
    }

    bool Object::operator==(const Object& rhs) const
    {
        return this->vao == rhs.vao;
    }

    bool Object::operator!=(ObjectHandle handle) const
    {
        return this->vao != handle;
    }

    bool Object::operator!=(const Object& rhs) const
    {
        return this->vao != rhs.vao;
    }

    std::size_t Object::add_buffer(std::unique_ptr<tz::gl::IBuffer> buffer)
    {
        auto find_result = std::find(this->buffers.begin(), this->buffers.end(), buffer);
        if(find_result == this->buffers.end())
        {
            // We don't contain this. Create it and return the index.
            this->buffers.push_back(std::move(buffer));
            return (this->buffers.size() - 1);
        }
        else
        {
            // We do contain this. Return the dist.
            return std::distance(this->buffers.begin(), find_result);
        }
    }

    std::size_t Object::add_index_buffer(std::unique_ptr<tz::gl::IBuffer> index_buffer)
    {
        std::size_t id = this->add_buffer(std::move(index_buffer));
        this->index_buffer_ids.push_back(id);
        return id;
    }

    void Object::format(std::size_t idx, tz::gl::Format fmt)
    {
        this->format_custom(idx, fmt.num_components, fmt.component_type, GL_FALSE, fmt.num_components * fmt.component_size, reinterpret_cast<const void*>(fmt.offset));
    }

    void Object::format_custom(std::size_t idx, GLint size, GLenum type, GLboolean normalised, GLsizei stride, const void* ptr)
    {
        this->bind_child(idx);
        std::size_t attrib_id = this->format_count++;
        glEnableVertexAttribArray(attrib_id);
        glVertexAttribPointer(attrib_id, size, type, normalised, stride, ptr);
    }

    tz::gl::IBuffer* Object::operator[](std::size_t idx)
    {
        topaz_assert(idx < this->size(), "tz::gl::Object::operator[", idx, "] was out of range. Size = ", this->size());
        return this->buffers[idx].get();
    }

    const tz::gl::IBuffer* Object::operator[](std::size_t idx) const
    {
        topaz_assert(idx < this->size(), "tz::gl::Object::operator[", idx, "] was out of range. Size = ", this->size());
        return this->buffers[idx].get();
    }

    void Object::bind_child(std::size_t idx) const
    {
        glBindVertexArray(this->vao);
        (*this)[idx]->bind();
    }

    void Object::erase(std::size_t idx)
    {
        topaz_assert(idx < this->size(), "tz::gl::Object::erase(", idx, "): Index ", idx, " was out of range! Size: ", this->size());
        this->buffers[idx] = nullptr;
    }

    std::unique_ptr<tz::gl::IBuffer> Object::release(std::size_t idx)
    {
        topaz_assert(idx < this->size(), "tz::gl::Object::release(", idx, "): Index ", idx, " was out of range! Size: ", this->size());
        return std::move(this->buffers[idx]);
    }

    void Object::render(std::size_t ibo_id) const
    {
        this->verify();
        this->bind_child(ibo_id);
        glDrawElements(GL_TRIANGLES, (*this)[ibo_id]->size() / sizeof(unsigned int), GL_UNSIGNED_INT, nullptr);
    }

    void Object::multi_render(std::size_t ibo_id, tz::gl::MDIDrawCommandList cmd_list) const
    {
        if(cmd_list.empty())
            return;
        this->verify();
        this->bind_child(ibo_id);
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, cmd_list.data(), cmd_list.size(), sizeof(tz::gl::gpu::DrawElementsIndirectCommand));
    }

    void Object::verify() const
    {
        topaz_assert(this->vao != 0, "tz::gl::Object::verify(): Verification failed");
    }

    void Object::verify_bound() const
    {
        topaz_assert(*this == bound::vao(), "tz::gl::Object::verify_bound(): Object is not bound!");
    }

    tz::gl::IBuffer* Object::bound_index_buffer()
    {
        for(std::size_t idx : this->index_buffer_ids)
        {
            tz::gl::IBuffer* buf = (*this)[idx];
            if(buf != nullptr && *buf == tz::gl::bound::vertex_buffer())
                return buf;
        }
        return nullptr;
    }

    namespace bound
    {
        int vao()
        {
            int vao_result;
            glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao_result);
            return vao_result;
        }
    }
}