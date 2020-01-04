//
// Created by Harrand on 25/12/2019.
//

#ifndef TOPAZ_GL_DRAW_COMMAND_HPP
#define TOPAZ_GL_DRAW_COMMAND_HPP
#include "glad/glad.h"
#include <cstdint>
#include <initializer_list>
#include <vector>

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

    class MDIDrawCommandList
    {
    public:
        using Command = gpu::DrawElementsIndirectCommand;
        MDIDrawCommandList(std::initializer_list<Command> cmds = {});
        const Command* data() const;
        Command* data();
        std::size_t size() const;
        bool empty() const;
        const Command& operator[](std::size_t idx) const;
        Command& operator[](std::size_t idx);
        std::size_t add(Command cmd);
        void erase(std::size_t idx);
    private:
        std::vector<Command> cmds;
    };
}

#endif // TOPAZ_GL_DRAW_COMMAND_HPP