#include "gl/index_snippet.hpp"
#include <algorithm>

namespace tz::gl
{
    IndexSnippet::IndexSnippet(std::size_t begin, std::size_t end, std::size_t offset): begin(begin), end(end), index_offset(offset){}

    std::size_t IndexSnippetList::size() const
    {
        return this->snippets.size();
    }

    bool IndexSnippetList::empty() const
    {
        return this->snippets.empty();
    }

    std::size_t IndexSnippetList::emplace_range(std::size_t begin, std::size_t end)
    {
        this->snippets.emplace_back(begin, end, 0u);
        return this->snippets.size() - 1;
    }

    std::size_t IndexSnippetList::emplace_range(std::size_t begin, std::size_t end, std::size_t index_offset)
    {
        this->snippets.emplace_back(begin, end, index_offset);
        return this->snippets.size() - 1;
    }

    tz::gl::MDIDrawCommandList IndexSnippetList::get_command_list() const
    {
        tz::gl::MDIDrawCommandList cmds;
        for(const IndexSnippet& snippet : this->snippets)
        {
            std::size_t first = snippet.begin;
            std::size_t second = snippet.end;
            std::size_t big = std::max(first, second);
            std::size_t small = std::min(first, second);
            std::size_t index_count = 1 + big - small;
            cmds.add({static_cast<GLuint>(index_count), 1, static_cast<GLuint>(small), static_cast<GLint>(snippet.index_offset), 0});
        }
        return cmds;
    }
}