//
// Created by Harrand on 25/12/2019.
//

#ifndef TOPAZ_GL_OBJECT_HPP
#define TOPAZ_GL_OBJECT_HPP
#include "gl/buffer.hpp"

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

        bool operator==(ObjectHandle handle) const;
        bool operator!=(ObjectHandle handle) const;
    private:
        void verify() const;
        ObjectHandle vao;
    };

    namespace bound
    {
        int vao();
    }
}

#endif // TOPAZ_GL_OBJECT_HPP