//
// Created by Harrand on 25/12/2019.
//

#include "gl/object.hpp"
#include "core/debug/assert.hpp"

namespace tz::gl
{
    Object::Object(): vao(0)
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

    bool Object::operator==(ObjectHandle handle) const
    {
        return this->vao == handle;
    }

    bool Object::operator!=(ObjectHandle handle) const
    {
        return this->vao != handle;
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