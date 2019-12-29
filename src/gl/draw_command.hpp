//
// Created by Harrand on 25/12/2019.
//

#ifndef TOPAZ_GL_DRAW_COMMAND_HPP
#define TOPAZ_GL_DRAW_COMMAND_HPP
#include "glad/glad.h"
#include <cstdint>

namespace tz::gl
{
    namespace gpu
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
}

#endif // TOPAZ_GL_DRAW_COMMAND_HPP