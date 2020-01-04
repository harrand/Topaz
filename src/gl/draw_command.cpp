//
// Created by Harrand on 25/12/2019.
//

#include "gl/draw_command.hpp"
#include "core/debug/assert.hpp"

namespace tz::gl
{
    MDIDrawCommandList::MDIDrawCommandList(std::initializer_list<Command> cmds): cmds(cmds){}

    const MDIDrawCommandList::Command* MDIDrawCommandList::data() const
    {
        return this->cmds.data();
    }

    MDIDrawCommandList::Command* MDIDrawCommandList::data()
    {
        return this->cmds.data();
    }

    std::size_t MDIDrawCommandList::size() const
    {
        return this->cmds.size();
    }

    bool MDIDrawCommandList::empty() const
    {
        return this->cmds.empty();
    }

    const MDIDrawCommandList::Command& MDIDrawCommandList::operator[](std::size_t idx) const
    {
        topaz_assert(idx < this->cmds.size(), "MDIDrawCommandList::operator[", idx, "]: Index ", idx, " was out of range! Size: ", this->size());
        return this->cmds[idx];
    }

    MDIDrawCommandList::Command& MDIDrawCommandList::operator[](std::size_t idx)
    {
        topaz_assert(idx < this->cmds.size(), "MDIDrawCommandList::operator[", idx, "]: Index ", idx, " was out of range! Size: ", this->size());
        return this->cmds[idx];
    }

    std::size_t MDIDrawCommandList::add(Command cmd)
    {
        this->cmds.push_back(cmd);
        return this->cmds.size() - 1;
    }

    void MDIDrawCommandList::erase(std::size_t idx)
    {
        topaz_assert(idx < this->cmds.size(), "MDIDrawCommandList::operator[", idx, "]: Index ", idx, " was out of range! Size: ", this->size());
        this->cmds.erase(this->cmds.begin() + idx);
    }
}