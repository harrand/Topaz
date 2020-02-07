#ifndef TOPAZ_MEMORY_VALMAP_HPP
#define TOPAZ_MEMORY_VALMAP_HPP
#include <map>
#include <unordered_map>

namespace tz::mem
{

    namespace detail
    {
        template<typename T>
        struct deref_less_than
        {
            bool operator()(const T* lhs, const T* rhs) const;
        };
    }

    template<typename K, typename V>
    class DeMap
    {
    public:
        using reference_type = std::pair<const K*, V*>;
        using ForwardMapType = std::map<K, V>;
        using BackwardMapType = std::map<const V*, const K*, detail::deref_less_than<V>>;
        using iterator = typename ForwardMapType::iterator;
        using const_iterator = typename ForwardMapType::const_iterator;
        DeMap() = default;
        template<typename... Args>
        reference_type emplace(Args&&... args);

        iterator begin();
        const_iterator cbegin() const;
        iterator end();
        const_iterator cend() const;

        iterator find_by_key(const K& key);
        const_iterator find_by_key(const K& key) const;

        bool contains_key(const K& key) const;

        iterator find_by_value(const V& value);
        const_iterator find_by_value(const V& value) const;
        
        bool contains_value(const V& value) const;

        void erase_key(const K& key);
        void erase_value(const V& value);

        const V& get_value(const K& key) const;
        void set_value(const K& key, V value);
        const K& get_key(const V& value) const;
        void set_key(const V& value, K key);
    private:
        typename BackwardMapType::iterator find_key_by_value(const V& value);
        typename BackwardMapType::const_iterator find_key_by_value(const V& value) const;

        BackwardMapType back;
        ForwardMapType ufor;
    };
}

#include "memory/demap.inl"
#endif // TOPAZ_MEMORY_VALMAP_HPP