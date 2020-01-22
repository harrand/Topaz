#include "core/debug/assert.hpp"

namespace tz::mem
{
    template<typename T>
    T* Block::get(std::size_t idx)
    {
        topaz_assert((idx + sizeof(T)) <= this->size(), "tz::mem::Block::operator<T>[", idx, "]: T at index ", idx, " is out of range! Size: ", this->size());
        char* beg = reinterpret_cast<char*>(this->begin);
        beg += static_cast<std::ptrdiff_t>(idx);
        return reinterpret_cast<T*>(beg);
    }

    template<typename T>
    const T* Block::get(std::size_t idx) const
    {
        topaz_assert((idx + sizeof(T)) <= this->size(), "tz::mem::Block::operator<T>[", idx, "]: T at index ", idx, " is out of range! Size: ", this->size());
        char* beg = reinterpret_cast<char*>(this->begin);
        beg += static_cast<std::ptrdiff_t>(idx);
        return reinterpret_cast<T*>(beg);
    }
}