//
// Created by Harrand on 25/12/2019.
//

#include "gl/object.hpp"
#include "core/debug/assert.hpp"
#include <algorithm>

namespace tz::gl
{
    Object::Object(): vao(0), buffers()
    {
        glGenVertexArrays(1, &this->vao);
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

    void Object::verify() const
    {
        topaz_assert(this->vao != 0, "tz::gl::Object::verify(): Verification failed");
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