#ifndef TOPAZ_GL_VK_COMMON_BASIC_LIST_HPP
#define TOPAZ_GL_VK_COMMON_BASIC_LIST_HPP
// TODO: Guard within TZ_VULKAN? Doesn't seem necessary.
#include <initializer_list>
#include <vector>
#include <algorithm>

namespace tz::gl::vk::common
{
    template<typename T, typename Allocator = std::allocator<T>>
    class BasicList
    {
    private:
        using UnderlyingList = std::vector<T, Allocator>;
        UnderlyingList elements;
    public:
        using Iterator = UnderlyingList::iterator;
        using ConstIterator = UnderlyingList::const_iterator;

        BasicList(std::initializer_list<T> elements):
        elements(elements){}

        BasicList() = default;

        T& front()
        {
            return this->elements.front();
        }

        const T& front() const
        {
            return this->elements.front();
        }

        T& back()
        {
            return this->elements.back();
        }

        const T& back() const
        {
            return this->elements.back();
        }

        void add(const T& element)
        {
            this->elements.push_back(element);
        }

        void add(T&& element)
        {
            this->elements.push_back(std::forward<T>(element));
        }

        void append(const BasicList<T>& other)
        {
            this->elements.insert(this->elements.end(), other.elements.begin(), other.elements.end());
        }

        bool contains(T element) const
        {
            return std::find(this->elements.begin(), this->elements.end(), element) != this->elements.end();
        }

        Iterator begin()
        {
            return this->elements.begin();
        }

        Iterator end()
        {
            return this->elements.end();
        }

        ConstIterator begin() const
        {
            return this->elements.begin();
        }

        ConstIterator end() const
        {
            return this->elements.end();
        }

        auto length() const
        {
            return this->elements.size();
        }

        const T* data() const
        {
            return this->elements.data();
        }

        Iterator erase(Iterator position)
        {
            return this->elements.erase(position);
        }

        Iterator erase(Iterator first, Iterator last)
        {
            return this->elements.erase(first, last);
        }
    };
}

#endif // TOPAZ_GL_VK_COMMON_BASIC_LIST_HPP