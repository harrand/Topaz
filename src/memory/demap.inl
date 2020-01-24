#include <utility>

namespace tz::mem
{
    namespace detail
    {
        template<typename T>
        bool deref_less_than<T>::operator()(const T* lhs, const T* rhs) const
        {
            return *lhs < *rhs;
        }
    }

    template<typename K, typename V>
    template<typename... Args>
    typename DeMap<K, V>::reference_type DeMap<K, V>::emplace(Args&&... args)
    {
        auto emplace_result = this->ufor.emplace(std::forward<Args>(args)...);
        auto iter_result = emplace_result.first;
        const K* key = &iter_result->first;
        V* val = &iter_result->second;
        this->back.emplace(val, key);
        return {key, val};
    }

    template<typename K, typename V>
    typename DeMap<K, V>::iterator DeMap<K, V>::begin()
    {
        return this->ufor.begin();
    }

    template<typename K, typename V>
    typename DeMap<K, V>::const_iterator DeMap<K, V>::cbegin() const
    {
        return this->ufor.cbegin();
    }

    template<typename K, typename V>
    typename DeMap<K, V>::iterator DeMap<K, V>::end()
    {
        return this->ufor.end();
    }

    template<typename K, typename V>
    typename DeMap<K, V>::const_iterator DeMap<K, V>::cend() const
    {
        return this->ufor.cend();
    }

    template<typename K, typename V>
    typename DeMap<K, V>::iterator DeMap<K, V>::find_by_key(const K& key)
    {
        return this->ufor.find(key);
    }

    template<typename K, typename V>
    typename DeMap<K, V>::const_iterator DeMap<K, V>::find_by_key(const K& key) const
    {
        return this->ufor.find(key);
    }

    template<typename K, typename V>
    bool DeMap<K, V>::contains_key(const K& key) const
    {
        return this->find_by_key(key) != this->cend();
    }

    template<typename K, typename V>
    typename DeMap<K, V>::iterator DeMap<K, V>::find_by_value(const V& value)
    {
        auto iter = this->find_key_by_value(value);
        if(iter == this->back.end())
            return this->end();
        else
        {
            K* key = iter->second;
            return this->find_by_key(*key);
        }
    }

    template<typename K, typename V>
    typename DeMap<K, V>::const_iterator DeMap<K, V>::find_by_value(const V& value) const
    {
        auto iter = this->find_key_by_value(value);
        if(iter == this->back.end())
            return this->cend();
        else
        {
            const K* key = iter->second;
            return this->find_by_key(*key);
        }
    }

    template<typename K, typename V>
    bool DeMap<K, V>::contains_value(const V& value) const
    {
        return this->find_by_value(value) != this->cend();
    }

    template<typename K, typename V>
    void DeMap<K, V>::erase_key(const K& key)
    {
        // Get exact address of the stored value
        V* value_ptr = &(this->ufor.find(key)->second);
        // Don't really need to erase from back first, but it's better practise.
        this->back.erase(value_ptr);
        // Now we can just erase the key.
        this->ufor.erase(key);
    }

    template<typename K, typename V>
    void DeMap<K, V>::erase_value(const V& value)
    {            
        // First, we get the address of the stored key.
        const K* key_ptr_to_erase = nullptr;
        const V* val_ptr_to_erase = nullptr;
        // boo linear :(
        for(const auto& [val_ptr, key_ptr] : this->back)
        {
            if(*val_ptr == value)
            {
                // Aha, we want this key!
                key_ptr_to_erase = key_ptr;
                val_ptr_to_erase = val_ptr;
            }
        }

        // Now we can do it similar as before:
        this->back.erase(val_ptr_to_erase);
        this->ufor.erase(*key_ptr_to_erase);
    }

    template<typename K, typename V>
    const V& DeMap<K, V>::get_value(const K& key) const
    {
        auto find_result = this->find_by_key(key);
        topaz_assert(find_result != this->cend(), "tz::mem::DeMap<K, V>::at_key(...) const: Key did not exist in map!");
        return find_result->second;
    }

    template<typename K, typename V>
    void DeMap<K, V>::set_value(const K& key, V value)
    {
        if(this->contains_key(key))
            this->erase_key(key);
        this->emplace(key, value);
    }

    template<typename K, typename V>
    const K& DeMap<K, V>::get_key(const V& value) const
    {
        auto find_result = this->find_by_value(value);
        topaz_assert(find_result != this->cend(), "tz::mem::DeMap<K, V>::at_value(...) const: Value did not exist in map!");
        return find_result->first;
    }

    template<typename K, typename V>
    void DeMap<K, V>::set_key(const V& value, K key)
    {
        if(this->contains_value(value))
            this->erase_value(value);
        this->emplace(key, value);
    }

    template<typename K, typename V>
    typename DeMap<K, V>::BackwardMapType::iterator DeMap<K, V>::find_key_by_value(const V& value)
    {
        return this->back.find(&value);
    }

    template<typename K, typename V>
    typename DeMap<K, V>::BackwardMapType::const_iterator DeMap<K, V>::find_key_by_value(const V& value) const
    {
        return this->back.find(&value);
    }
}