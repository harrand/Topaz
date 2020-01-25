#ifndef TOPAZ_MEMORY_VALMAP_HPP
#define TOPAZ_MEMORY_VALMAP_HPP
#include <map>
#include <unordered_map>
#include <type_traits>

namespace tz::mem
{

    namespace detail
    {
        template<typename T>
        struct deref_less_than
        {
            bool operator()(const T* lhs, const T* rhs) const;
        };

        template<typename A, typename B>
        using RefPair = std::pair<const A*, B*>;

        template<typename A, typename B>
        using CRefPair = std::pair<const A*, const B*>;
    }

    /**
     * Like an std::map, but is sorted by value instead of by key.
     * 
     * This custom data structure sacrifices memory efficiency in favour of runtime efficiency.
     */
    template<typename K, typename V>
    class DeMap
    {
    public:
        using reference_type = std::pair<const K*, V*>;
        using ForwardMapType = std::map<K, V>;
        using BackwardMapType = std::map<const V*, const K*, detail::deref_less_than<V>>;
        
        class iterator
        {
            // Just like ForwardMapType::iterator, but must be ordered depending on BackwardMapType.
        public:
            iterator(DeMap<K, V>& parent);
            iterator(DeMap<K, V>& parent, std::ptrdiff_t idx);
            using RefPair = detail::RefPair<K, V>;
            using CRefPair = detail::CRefPair<K, V>;

            RefPair operator*();
            CRefPair operator*() const;

            bool operator==(const iterator& rhs) const;
            bool operator!=(const iterator& rhs) const;

            iterator& operator++();
            iterator& operator++(int);
            iterator& operator+=(std::ptrdiff_t offset);
            iterator operator+(std::ptrdiff_t offset) const;
        private:
            DeMap<K, V>& parent;
            std::ptrdiff_t idx;
        };

        class const_iterator
        {
            // Just like ForwardMapType::iterator, but must be ordered depending on BackwardMapType.
        public:
            const_iterator(const DeMap<K, V>& parent);
            const_iterator(const DeMap<K, V>& parent, std::ptrdiff_t idx);
            using CRefPair = detail::CRefPair<K, V>;

            CRefPair operator*();
            CRefPair operator*() const;

            bool operator==(const const_iterator& rhs) const;
            bool operator!=(const const_iterator& rhs) const;

            const_iterator& operator++();
            const_iterator& operator++(int);
            const_iterator& operator+=(std::ptrdiff_t offset);
            const_iterator operator+(std::ptrdiff_t offset) const;
        private:
            const DeMap<K, V>& parent;
            std::ptrdiff_t idx;
        };
        
        /**
         * Construct an empty demap.
         */
        DeMap() = default;
        /**
         * Construct a key-value pair in-place within the demap.
         *
         * Complexity: O(log n) with respect to the size of the container.
         * @tparam Args Argument types used to construct the key and value objects.
         * @param args Argument values used to construct the key and value objects.
         * @return Pair of pointers to the constructed key-value pair. Note that the key is immutable.
         */
        template<typename... Args>
        reference_type emplace(Args&&... args);

        /**
         * Retrieve an iterator to the beginning of the demap.
         * 
         * This will be directed to the entry with the "lowest" key.
         * @return Iterator to lowest entry.
         */
        iterator begin();
        /**
         * Retrieve an immutable iterator to the beginning of the demap.
         * 
         * This will be directed to the entry with the "lowest" key.
         * @return Iterator to lowest entry.
         */
        const_iterator begin() const;
        /**
         * Retrieve an immutable iterator to the beginning of the demap.
         * 
         * This will be directed to the entry with the "lowest" key.
         * @return Iterator to lowest entry.
         */
        const_iterator cbegin() const;
        /**
         * Retrieve an iterator to the end of the demap.
         * 
         * This will be directed to the entry with the "highest" key.
         * @return Iterator to highest entry.
         */
        iterator end();
        /**
         * Retrieve an immutable iterator to the end of the demap.
         * 
         * This will be directed to the entry with the "highest" key.
         * @return Iterator to highest entry.
         */
        const_iterator end() const;
        /**
         * Retrieve an immutable iterator to the end of the demap.
         * 
         * This will be directed to the entry with the "highest" key.
         * @return Iterator to highest entry.
         */
        const_iterator cend() const;
        /**
         * Erase all elements within the demap.
         * Invalidates all iterators and references.
         */
        void clear();
        /**
         * Find an entry by key.
         * 
         * Complexity: O(log n) with respect to the size of the container.
         * @param key Key whose corresponding entry should be retrieved.
         */
        iterator find_by_key(const K& key);
        /**
         * Find an immutable entry by key.
         * 
         * Complexity: O(log n) with respect to the size of the container.
         * @param key Key whose corresponding entry should be retrieved.
         */
        const_iterator find_by_key(const K& key) const;
        /**
         * Query as to whether an entry exists within the demap with a key equal to the given value.
         * 
         * Complexity: O(log n) with respect to the size of the container.
         * @param key Key whose value should be checked for existence.
         * @return True if the key exists, otherwise false.
         */
        bool contains_key(const K& key) const;
        /**
         * Find an entry by value.
         * 
         * Complexity: O(log n) with respect to the size of the container.
         * @param value Value whose corresponding entry should be retrieved.
         */
        iterator find_by_value(const V& value);
        /**
         * Find an immutable entry by value.
         * 
         * Complexity: O(log n) with respect to the size of the container.
         * @param value Value whose corresponding entry should be retrieved.
         */
        const_iterator find_by_value(const V& value) const;
        /**
         * Query as to whether an entry exists within the demap with the given value.
         * 
         * Complexity: O(log n) with respect to the size of the container.
         * @param value Value which should be checked for existence.
         * @return True if the value exists, otherwise false.
         */
        bool contains_value(const V& value) const;
        /**
         * Erase an entry with the given key, if one exists. Otherwise, does nothing.
         * 
         * Complexity: O(1) amortised. Should always beat O(n).
         * @param key Key whose entry need be erased if it exists.
         */
        void erase_key(const K& key);
        /**
         * Erase an entry with the given value, if one exists. Otherwise, does nothing.
         * 
         * Complexity: O(n)
         * @param value Value whose entry need be erased if it exists.
         */
        void erase_value(const V& value);
        /**
         * Retrieve a value corresponding to the given key.
         * 
         * Precondition: An entry exists with the given key. Otherwise, this will assert and invoke UB.
         * Complexity: O(log n) with respect to the size of the container.
         * @param key Key whose value to retrieve.
         * @return Value corresponding to the given key.
         */
        const V& get_value(const K& key) const;
        /**
         * Set the key-value pair.
         * 
         * Note: If this key already exists in the demap, its value will be replaced by this.
         * Complexity: O(log n) with respect to the size of the container.
         * @param key Key to add.
         * @param value Value to add.
         */
        void set_value(const K& key, V value);
        /**
         * Retrieve a key corresponding to the given value.
         * 
         * Precondition: An entry exists with the given value. Otherwise, this will assert and invoke UB.
         * Complexity: O(log n) with respect to the size of the container.
         * @param value Value whose key to retrieve.
         * @return Key corresponding to the given value.
         */
        const K& get_key(const V& value) const;
        /**
         * Set the key-value pair.
         * 
         * Note: If this key already exists in the demap, its value will be replaced by this.
         * Complexity: O(log n) with respect to the size of the container.
         * @param value Value to add.
         * @param key Key to add.
         */
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