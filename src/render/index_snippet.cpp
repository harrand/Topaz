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
        this->ranges.emplace_back(begin, end);
        return this->ranges.size() - 1;
    }

    std::vector<tz::gl::gpu::DrawElementsIndirectCommand> IndexSnippet::get_command_list() const
    {
        std::vector<tz::gl::gpu::DrawElementsIndirectCommand> cmds;
        for(const IndexRange& range : this->ranges)
        {
            std::size_t big = std::max(range.first, range.second);
            std::size_t small = std::min(range.first, range.second);
            std::size_t index_count = big - small;
            cmds.push_back({static_cast<GLuint>(index_count), static_cast<GLuint>(index_count / 3), static_cast<GLuint>(small), 0, 0});
        }
        return cmds;
    }
}