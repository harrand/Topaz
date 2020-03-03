#ifndef TOPAZ_GL_DRAW_COMMAND_HPP
#define TOPAZ_GL_DRAW_COMMAND_HPP
#include "glad/glad.h"
#include <cstdint>
#include <initializer_list>
#include <vector>

namespace tz::gl
{
    /**
     * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
     * @{
     */

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

    /**
     * Represents zero or more MDI (gpu::DrawElementsIndirectCommand) commands.
     * 
     * MDI command lists can be used to draw several different meshes in a single draw-call, which drastically reduces driver overhead.
     */
    class MDIDrawCommandList
    {
    public:
        using Command = gpu::DrawElementsIndirectCommand;
        /**
         * Construct a command list from a given set of commands.
         * @cmds Command list to begin with.
         */
        MDIDrawCommandList(std::initializer_list<Command> cmds = {});
        /**
         * Retrieve a pointer to the first element in the list.
         * 
         * Note: The commands are guaranteed to be contiguous in memory, and be of size this->size().
         * @return Pointer to immutable command list.
         */
        const Command* data() const;
        /**
         * Retrieve a pointer to the first element in the list.
         * 
         * Note: The commands are guaranteed to be contiguous in memory, and be of size this->size().
         * @return Pointer to mutable command list.
         */
        Command* data();
        /**
         * Retrieve the number of commands in the list.
         * 
         * @return Size of this list, in number of elements.
         */
        std::size_t size() const;
        /**
         * Query if the list is empty.
         * 
         * @return True if this->size() == 0. False otherwise.
         */
        bool empty() const;
        /**
         * Retrieve the Command at the given index.
         * 
         * Precondition: idx must be in-range (idx < this->size()). Otherwise, this will assert and invoke UB.
         * @param idx Index in the list whose command should be retrieved.
         * @return Immutable reference to the command at the given index.
         */
        const Command& operator[](std::size_t idx) const;
        /**
         * Retrieve the Command at the given index.
         * 
         * Precondition: idx must be in-range (idx < this->size()). Otherwise, this will assert and invoke UB.
         * @param idx Index in the list whose command should be retrieved.
         * @return Mutable reference to the command at the given index.
         */
        Command& operator[](std::size_t idx);
        /**
         * Add the given command to the end of the list.
         * 
         * @param cmd Command to add to the end of the list.
         * @return Index corresponding to the newly-created command in the list.
         */
        std::size_t add(Command cmd);
        /**
         * Erase the command corresponding to the given index.
         * 
         * Note: This may invalidate indices for all existing commands in the list.
         * TODO: Make indices stable by preventing the list from ever shrinking.
         * Precondition: idx must be in-range (idx < this->size()). Otherwise, this will assert and invoke UB.
         * @param idx Index whose corresponding command should be deleted.
         */
        void erase(std::size_t idx);
    private:
        std::vector<Command> cmds;
    };

    /**
     * @}
     */
}

#endif // TOPAZ_GL_DRAW_COMMAND_HPP