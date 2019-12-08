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

template<typename T>
DynamicTree<T>::DynamicTree(): parent(nullptr), children() {}

template<typename T>
DynamicTree<T>::DynamicTree(const T* parent): parent(parent), children(){}

template<typename T>
const T* DynamicTree<T>::get_parent() const
{
	return this->parent;
}

template<typename T>
const T* DynamicTree<T>::get_child(std::size_t child_index) const
{
	return this->children[child_index].get();
}

template<typename T>
template<typename... Args>
bool DynamicTree<T>::set_child(std::size_t child_index, Args&&... args)
{
	bool requires_expansion = child_index >= this->get_dimensions();
	if(requires_expansion)
	{
		this->children.resize(child_index + 1, nullptr);
	}
	this->children[child_index] = std::make_unique<T>(std::forward<Args>(args)...);
    return requires_expansion;
}

template<typename T>
std::size_t DynamicTree<T>::get_dimensions() const
{
	return this->children.size();
}