
namespace tz::gl
{
    template<tz::gl::BufferType Type, typename... Args>
    std::size_t Object::emplace_buffer(Args&&... args)
    {
        this->bind();
        auto buffer_ptr = std::make_unique<tz::gl::Buffer<Type>>(std::forward<Args>(args)...);
        if constexpr(Type == tz::gl::BufferType::Index)
            return this->add_index_buffer(std::move(buffer_ptr));
        else
            return this->add_buffer(std::move(buffer_ptr));
    }

    template<tz::gl::BufferType Type>
    tz::gl::Buffer<Type>* Object::get(std::size_t idx)
    {
        return static_cast<tz::gl::Buffer<Type>*>((*this)[idx]);
    }

    template<tz::gl::BufferType Type>
    const tz::gl::Buffer<Type>* Object::get(std::size_t idx) const
    {
        return static_cast<const tz::gl::Buffer<Type>*>((*this)[idx]);
    }

    template<tz::gl::BufferType Type, typename... Args>
    void Object::set(std::size_t idx, Args&&... args)
    {
        this->verify();
        topaz_assert(idx < this->size(), "tz::gl::Object::set(", idx, ", ...): Index given to set was not in range. Given an index ", idx, " which must be less-than (", this->size(), ")");
        // TODO: Implement
        this->erase(idx);
        topaz_assert((*this)[idx] == nullptr, "tz::gl::Object::set(", idx, ", ...): Previous element at the index ", idx, " wasn't erased properly.");
        this->buffers[idx] = std::make_unique<tz::gl::Buffer<Type>>(std::forward<Args>(args)...);
    }

    template<typename IterT>
    void Object::render(IterT indices_begin, IterT indices_end) const
    {
        this->verify();
        this->verify_bound();
        std::size_t index_buffer_count = std::distance(indices_begin, indices_end);
        bool multi_draw = (index_buffer_count > 1);
        for(IterT i = indices_begin; i < indices_end; i++)
        {
            std::size_t index = *i;
            // Ensure it's an index buffer as far as we're concerned.
            {
                auto find_result = std::find(this->index_buffer_ids.begin(), this->index_buffer_ids.end(), index);
                topaz_assert(find_result != this->index_buffer_ids.end(), "tz::gl::Object::render(...): One of the handle IDs (", index, ") in collection was not marked as an index-buffer!");
            }
            // We can safely interpret it as an index-buffer.
            if(multi_draw)
            {
                // TODO: MD Support
                topaz_assert(false, "tz::gl::Object::render(...): Multi-draw not yet implemented!");
            }
            else
            {
                //this->bind_child(index);
                std::size_t num_indices = this->get<tz::gl::BufferType::Index>(index)->size() / sizeof(unsigned int);
                //glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(num_indices), GL_UNSIGNED_INT, nullptr);
                glDrawArrays(GL_TRIANGLES, 0, 3);
            }
        }
    }
}