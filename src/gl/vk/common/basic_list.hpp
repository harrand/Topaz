#ifndef TOPAZ_GL_VK_COMMON_BASIC_LIST_HPP
#define TOPAZ_GL_VK_COMMON_BASIC_LIST_HPP
// TODO: Guard within TZ_VULKAN? Doesn't seem necessary.
#include <initializer_list>
#include <vector>
#include <algorithm>

namespace tz::gl::vk::common
{
    template<typename T>
    class BasicList
    {
    public:
        BasicList(std::initializer_list<T> elements = {}):
        elements(elements){}

        void add(T element)
        {
            this->elements.push_back(element);
        }

        void append(const BasicList<T>& other)
        {
            this->elements.insert(this->elements.end(), other.elements.begin(), other.elements.end());
        }

        bool contains(T element) const
        {
            return std::find(this->elements.begin(), this->elements.end(), element) != this->elements.end();
        }

        auto begin()
        {
            return this->elements.begin();
        }

        auto end()
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
    private:
        std::vector<T> elements;
    };
}

#endif // TOPAZ_GL_VK_COMMON_BASIC_LIST_HPP