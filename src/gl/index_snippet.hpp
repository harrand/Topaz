#ifndef TOPAZ_GL_INDEX_SNIPPET_HPP
#define TOPAZ_GL_INDEX_SNIPPET_HPP
#include <cstdint>
#include <utility>
#include <vector>
#include "gl/draw_command.hpp"
#include "gl/manager.hpp"

namespace tz::gl
{
    /**
     * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
     * @{
     */

    struct IndexSnippet
    {
        IndexSnippet(std::size_t begin, std::size_t end, std::size_t offset);
        IndexSnippet(const tz::gl::Manager& manager, tz::gl::Manager::Handle mesh_handle);

        gpu::DrawElementsIndirectCommand mdi() const;
        
        std::size_t begin;
        std::size_t end;
        std::size_t index_offset;
    };
    /**
     * IndexSnippetLists contain ranges of indices, where each index is an offset into an existing index-buffer.
     * 
     * In other words, these indices are indices into the index-buffers indices. Unfortunately the inner-workings are a bit complex.
     * For clarity, see the topaz_multi_draw_demo's code as a useful example.
     */
    class IndexSnippetList
    {
    public:
        /**
         * Construct a snippet corresponding to the ibo_index.
         * 
         * Note: This snippet need not be aware of the Object corresponding to the ibo_index. This is confusing but does increase flexibility of this object.
         * Precondition: The index must be valid in the eyes of that object with which this snippet will be rendered with.
         * @param ibo_index Index buffer corresponding to the ranges within this snippet.
         */
        IndexSnippetList() = default;
        /**
         * Retrieve the number of ranges stored within this snippet.
         * @return Number of ranges in this snippet.
         */
        std::size_t size() const;
        /**
         * Query as to whether this snippet contains any ranges.
         * @return True if this->size() == 0. False otherwise.
         */
        bool empty() const;
        /**
         * Add a new index-range into this snippet.
         * @param begin Index that the sequence begins (inclusive).
         * @param end Index that the sequence ends (inclusive).
         * @return Index referring to this position of this range in the snippet.
         */
        std::size_t emplace_range(std::size_t begin, std::size_t end);
        std::size_t emplace_range(std::size_t begin, std::size_t end, std::size_t index_offset);
        std::size_t emplace_range(const tz::gl::Manager& manager, tz::gl::Manager::Handle mesh_handle);
        /**
         * Using the current ranges within this command-list, retrieve an MDI command list which can be used in a render-invocation.
         * @return Render-ready MDI command list.
         */
        tz::gl::MDIDrawCommandList get_command_list() const;
    private:
        std::vector<IndexSnippet> snippets;
    };

    /**
     * @}
     */
}

#endif // TOPAZ_GL_INDEX_SNIPPET_HPP