//
// Created by Harrand on 25/12/2019.
//

#ifndef TOPAZ_GL_DRAW_COMMAND_HPP
#define TOPAZ_GL_DRAW_COMMAND_HPP
#include "glad/glad.h"

namespace tz::gl
{
    namespace detail
    {
        struct DrawArraysIndirectCommand
        {
            GLuint count;
            GLuint prim_count;
            GLuint first;
            GLuint base_instance;
        };

        struct DrawElementsIndirectCommand
        {
            GLuint count;
            GLuint prim_count;
            GLuint first_index;
            GLint base_vertex;
            GLuint base_instance;
        };

    }

    class IDrawCommand
    {
        virtual void invoke() const = 0;
        void operator()() const;
    };
}

#endif // TOPAZ_GL_DRAW_COMMAND_HPP