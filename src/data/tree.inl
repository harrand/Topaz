template<typename T, std::size_t dimension>
Tree<T, dimension>::Tree(): parent(nullptr), children{}{}

template<typename T, std::size_t dimension>
const T* Tree<T, dimension>::get_parent() const
{
    return this->parent;
}

template<typename T, std::size_t dimension>
const T* Tree<T, dimension>::get_child(std::size_t child_index) const
{
    if(child_index >= dimension)
        return nullptr;
    return this->children[child_index].get();
}