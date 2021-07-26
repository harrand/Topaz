#ifndef TOPAZ_GL_VK_COMMON_POLYMORPHIC_LIST_HPP
#define TOPAZ_GL_VK_COMMON_POLYMORPHIC_LIST_HPP
#include "core/containers/basic_list.hpp"
#include <memory>

namespace tz
{
    /**
	 * \addtogroup tz_core Topaz Core Library (tz)
	 * A collection of platform-agnostic core interfaces.
	 * @{
	 */
    template<typename T>
    class InterfaceIterator;

    template<typename T, typename Allocator = std::allocator<std::unique_ptr<T>>>
    class PolymorphicList
    {
    public:
        using Pointer = T*;
        using Reference = T&;
        using Iterator = InterfaceIterator<T>;

        PolymorphicList() = default;
        template<typename Derived, typename... Args>
        Reference emplace(Args&&... args);
        Iterator begin();
        Iterator end();
        Iterator begin() const;
        Iterator end() const;
        std::size_t size() const;
        const T& operator[](std::size_t idx) const;
        T& operator[](std::size_t idx);
    private:
        using SmartPointer = std::unique_ptr<T>;
        using SmartPointerConst = std::unique_ptr<const T>;
        BasicList<SmartPointer, Allocator> elements;
    };

    template<typename T>
    class InterfaceIterator
    {
    public:
        InterfaceIterator(const std::unique_ptr<T>* loc);
        
        T& operator*();
        const T& operator*() const;
        T* operator->();
        const T* operator->() const;
        InterfaceIterator<T>& operator++();
        InterfaceIterator<T>& operator++(int);
        bool operator==(const InterfaceIterator<T>& rhs) const;
    private:
        const std::unique_ptr<T>* loc;
    };
    /**
	 * @}
	 */
}

#include "core/containers/polymorphic_list.inl"
#endif // TOPAZ_GL_VK_COMMON_POLYMORPHIC_LIST_HPP