#include "render/index_snippet.hpp"
#include <algorithm>

namespace tz::render
{
    IndexSnippet::IndexSnippet(std::size_t ibo_index): ibo_index(ibo_index){}

    void IndexSnippet::set_buffer(std::size_t ibo_index)
    {
        this->ibo_index = ibo_index;
    }

    std::size_t IndexSnippet::size() const
    {
        return this->ranges.size();
    }

    bool IndexSnippet::empty() const
    {
        return this->ranges.empty();
    }

    std::size_t IndexSnippet::emplace_range(std::size_t begin, std::size_t end)
    {
        this->ranges.emplace_back(IndexRange{begin, end}, 0u);
        return this->ranges.size() - 1;
    }

    std::size_t IndexSnippet::emplace_range(std::size_t begin, std::size_t end, std::size_t index_offset)
    {
        this->ranges.emplace_back(IndexRange{begin, end}, index_offset);
        return this->ranges.size() - 1;
    }

    tz::gl::MDIDrawCommandList IndexSnippet::get_command_list() const
    {
        tz::gl::MDIDrawCommandList cmds;
        for(const IndexRangeElement& element : this->ranges)
        {
            const IndexRange& range = element.range;
            std::size_t big = std::max(range.first, range.second);
            std::size_t small = std::min(range.first, range.second);
            std::size_t index_count = 1 + big - small;
            cmds.add({static_cast<GLuint>(index_count), static_cast<GLuint>(index_count / 3), static_cast<GLuint>(small), static_cast<GLint>(element.index_offset), 0});
        }
        return cmds;
    }
}