#include <utility>

namespace tz
{
    template<typename T, typename Allocator>
    template<typename Derived, typename... Args>
    PolymorphicList<T, Allocator>::Reference PolymorphicList<T, Allocator>::emplace(Args&&... args)
    {
        auto ptr = static_cast<SmartPointer>(std::make_unique<Derived>(std::forward<Args>(args)...));
        this->elements.add(std::move(ptr));
        return *this->elements.back();
    }

    template<typename T, typename Allocator>
    PolymorphicList<T, Allocator>::Iterator PolymorphicList<T, Allocator>::begin()
    {
        return {this->elements.data()};
    }

    template<typename T, typename Allocator>
    PolymorphicList<T, Allocator>::Iterator PolymorphicList<T, Allocator>::end()
    {
        return {this->elements.data() + this->elements.length()};
    }

    template<typename T, typename Allocator>
    PolymorphicList<T, Allocator>::Iterator PolymorphicList<T, Allocator>::begin() const
    {
        return {this->elements.data()};
    }

    template<typename T, typename Allocator>
    PolymorphicList<T, Allocator>::Iterator PolymorphicList<T, Allocator>::end() const
    {
        return {this->elements.data() + this->elements.length()};
    }

    template<typename T, typename Allocator>
    std::size_t PolymorphicList<T, Allocator>::size() const
    {
        return this->elements.length();
    }

    template<typename T, typename Allocator>
    const T& PolymorphicList<T, Allocator>::operator[](std::size_t idx) const
    {
        tz_assert(idx < this->size(), "Index %zu out of range, size = %zu", idx, this->size());
        return *this->elements[idx];
    }
        
    template<typename T, typename Allocator>
    T& PolymorphicList<T, Allocator>::operator[](std::size_t idx)
    {
        tz_assert(idx < this->size(), "Index %zu out of range, size = %zu", idx, this->size());
        return *this->elements[idx];
    }

    template<typename T>
    InterfaceIterator<T>::InterfaceIterator(const std::unique_ptr<T>* loc):
    loc(loc)
    {}

    template<typename T>
    T& InterfaceIterator<T>::operator*()
    {
        return **this->loc;
    }

    template<typename T>
    const T& InterfaceIterator<T>::operator*() const
    {
        return **this->loc;
    }

    template<typename T>
    T* InterfaceIterator<T>::operator->()
    {
        return this->loc->get();
    }

    template<typename T>
    const T* InterfaceIterator<T>::operator->() const
    {
        return this->loc->get();
    }

    template<typename T>
    InterfaceIterator<T>& InterfaceIterator<T>::operator++()
    {
        loc++;
        return *this;
    }

    template<typename T>
    InterfaceIterator<T>& InterfaceIterator<T>::operator++(int)
    {
        ++loc;
        return *this;
    }

    template<typename T>
    bool InterfaceIterator<T>::operator==(const InterfaceIterator<T>& rhs) const
    {
        return this->loc->get() == rhs.loc->get();
    }
}