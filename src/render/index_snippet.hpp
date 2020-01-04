#ifndef TOPAZ_RENDER_INDEX_SNIPPET_HPP
#define TOPAZ_RENDER_INDEX_SNIPPET_HPP
#include <cstdint>
#include <utility>
#include <vector>
#include "gl/draw_command.hpp"

namespace tz::render
{
    class IndexSnippet
    {
    public:
        IndexSnippet(std::size_t ibo_index);
        void set_buffer(std::size_t ibo_index);
        std::size_t size() const;
        bool empty() const;
        std::size_t emplace_range(std::size_t begin, std::size_t end);
        // really ugly but can refactor this later.
        std::vector<tz::gl::gpu::DrawElementsIndirectCommand> get_command_list() const;
    private:
        using IndexRange = std::pair<std::size_t, std::size_t>;
        std::size_t ibo_index;
        std::vector<IndexRange> ranges;
    };
}

#endif // TOPAZ_RENDER_INDEX_SNIPPET_HPP