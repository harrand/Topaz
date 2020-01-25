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
    DeMap<K, V>::iterator::iterator(DeMap<K, V>& parent): parent(parent), idx(0){}

    template<typename K, typename V>
    DeMap<K, V>::iterator::iterator(DeMap<K, V>& parent, std::ptrdiff_t idx): parent(parent), idx(idx){}

    template<typename K, typename V>
    typename DeMap<K, V>::iterator::RefPair DeMap<K, V>::iterator::operator*()
    {
        auto back_iter = this->parent.back.begin();
        std::advance(back_iter, this->idx);
        const K* ck_ptr = back_iter->second;
        V& v_ref = this->parent.ufor[*ck_ptr];
        return RefPair{ck_ptr, &v_ref};
    }

    template<typename K, typename V>
    typename DeMap<K, V>::iterator::CRefPair DeMap<K, V>::iterator::operator*() const
    {
        auto back_iter = this->parent.back.begin();
        std::advance(back_iter, this->idx);
        const K* ck_ptr = back_iter->second;
        const V* cv_ptr = back_iter->first;
        return {ck_ptr, cv_ptr};
    }

    template<typename K, typename V>
    bool DeMap<K, V>::iterator::operator==(const DeMap<K, V>::iterator& rhs) const
    {
        return this->idx == rhs.idx;
    }

    template<typename K, typename V>
    bool DeMap<K, V>::iterator::operator!=(const DeMap<K, V>::iterator& rhs) const
    {
        return this->idx != rhs.idx;
    }

    template<typename K, typename V>
    typename DeMap<K, V>::iterator::iterator& DeMap<K, V>::iterator::operator++()
    {
        ++(this->idx);
        return *this;
    }

    template<typename K, typename V>
    typename DeMap<K, V>::iterator::iterator& DeMap<K, V>::iterator::operator++(int)
    {
        this->idx++;
        return *this;
    }

    template<typename K, typename V>
    typename DeMap<K, V>::iterator::iterator& DeMap<K, V>::iterator::operator+=(std::ptrdiff_t offset)
    {
        this->idx += offset;
        return *this;
    }

    template<typename K, typename V>
    typename DeMap<K, V>::iterator::iterator DeMap<K, V>::iterator::operator+(std::ptrdiff_t offset) const
    {
        iterator copy = *this;
        return copy += offset;
    }

    template<typename K, typename V>
    DeMap<K, V>::const_iterator::const_iterator(const DeMap<K, V>& parent): parent(parent), idx(0){}

    template<typename K, typename V>
    DeMap<K, V>::const_iterator::const_iterator(const DeMap<K, V>& parent, std::ptrdiff_t idx): parent(parent), idx(idx){}

    template<typename K, typename V>
    typename DeMap<K, V>::const_iterator::CRefPair DeMap<K, V>::const_iterator::operator*()
    {
        auto back_iter = this->parent.back.begin();
        std::advance(back_iter, this->idx);
        const K* ck_ptr = back_iter->second;
        const V& v_ref = this->parent.ufor.at(*ck_ptr);
        return CRefPair{ck_ptr, &v_ref};
    }

    template<typename K, typename V>
    typename DeMap<K, V>::const_iterator::CRefPair DeMap<K, V>::const_iterator::operator*() const
    {
        auto back_iter = this->parent.back.begin();
        std::advance(back_iter, this->idx);
        const K* ck_ptr = back_iter->second;
        const V* cv_ptr = back_iter->first;
        return {ck_ptr, cv_ptr};
    }

    template<typename K, typename V>
    bool DeMap<K, V>::const_iterator::operator==(const DeMap<K, V>::const_iterator& rhs) const
    {
        return this->idx == rhs.idx;
    }

    template<typename K, typename V>
    bool DeMap<K, V>::const_iterator::operator!=(const DeMap<K, V>::const_iterator& rhs) const
    {
        return this->idx != rhs.idx;
    }

    template<typename K, typename V>
    typename DeMap<K, V>::const_iterator::const_iterator& DeMap<K, V>::const_iterator::operator++()
    {
        ++(this->idx);
        return *this;
    }

    template<typename K, typename V>
    typename DeMap<K, V>::const_iterator::const_iterator& DeMap<K, V>::const_iterator::operator++(int)
    {
        this->idx++;
        return *this;
    }

    template<typename K, typename V>
    typename DeMap<K, V>::const_iterator::const_iterator& DeMap<K, V>::const_iterator::operator+=(std::ptrdiff_t offset)
    {
        this->idx += offset;
        return *this;
    }

    template<typename K, typename V>
    typename DeMap<K, V>::const_iterator::const_iterator DeMap<K, V>::const_iterator::operator+(std::ptrdiff_t offset) const
    {
        const_iterator copy = *this;
        return copy += offset;
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
        auto iter = this->back.begin();
        return iterator{*this, 0};
    }

    template<typename K, typename V>
    typename DeMap<K, V>::const_iterator DeMap<K, V>::begin() const
    {
        auto iter = this->back.begin();
        return const_iterator{*this, 0};
    }

    template<typename K, typename V>
    typename DeMap<K, V>::const_iterator DeMap<K, V>::cbegin() const
    {
        return const_iterator{*this, 0};
    }

    template<typename K, typename V>
    typename DeMap<K, V>::iterator DeMap<K, V>::end()
    {
        auto iter = this->back.end();
        return iterator{*this, static_cast<std::ptrdiff_t>(this->back.size())};
    }

    template<typename K, typename V>
    typename DeMap<K, V>::const_iterator DeMap<K, V>::end() const
    {
        return const_iterator{*this, static_cast<std::ptrdiff_t>(this->back.size())};
    }

    template<typename K, typename V>
    typename DeMap<K, V>::const_iterator DeMap<K, V>::cend() const
    {
        return const_iterator{*this, static_cast<std::ptrdiff_t>(this->back.size())};
    }

    template<typename K, typename V>
    void DeMap<K, V>::clear()
    {
        this->ufor.clear();
        this->back.clear();
    }

    template<typename K, typename V>
    typename DeMap<K, V>::iterator DeMap<K, V>::find_by_key(const K& key)
    {
        topaz_assert(this->contains_key(key), "tz::mem::DeMap<K, V>::find_by_key(key): Key did not exist in the demap!");
        const V* val_ptr = &this->ufor.at(key);
        auto val_find = this->back.find(val_ptr);
        return iterator{*this, std::distance(this->back.begin(), val_find)};
    }

    template<typename K, typename V>
    typename DeMap<K, V>::const_iterator DeMap<K, V>::find_by_key(const K& key) const
    {
        topaz_assert(this->contains_key(key), "tz::mem::DeMap<K, V>::find_by_key(key): Key did not exist in the demap!");
        const V* val_ptr = &this->ufor.at(key);
        auto val_find = this->back.find(val_ptr);
        return const_iterator{*this, std::distance(this->back.begin(), val_find)};
    }

    template<typename K, typename V>
    bool DeMap<K, V>::contains_key(const K& key) const
    {
        return this->ufor.find(key) != this->ufor.end();
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
        if(!this->contains_key(key))
            return;
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
        // If nothing procs above, these will still be nullptr.
        // cppreference: 3) Removes the element (if one exists) with the key equivalent to key.
        // So we're fine to let these erasures happen and it will do nothing.

        // Now we can do it similar as before:
        this->back.erase(val_ptr_to_erase);
        this->ufor.erase(*key_ptr_to_erase);
    }

    template<typename K, typename V>
    const V& DeMap<K, V>::get_value(const K& key) const
    {
        auto find_result = this->find_by_key(key);
        topaz_assert(find_result != this->cend(), "tz::mem::DeMap<K, V>::at_key(...) const: Key did not exist in map!");
        return *((*find_result).second);
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
        return *((*find_result).first);
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