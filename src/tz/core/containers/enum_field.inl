#include <algorithm>

namespace tz
{
	template<tz::enum_class E>
	constexpr EnumField<E>::EnumField(E type):
	elements(type)
	{}

	template<tz::enum_class E>
	constexpr EnumField<E>::EnumField(std::initializer_list<E> types):
	elements(types)
	{}

	template<tz::enum_class E>
	bool EnumField<E>::contains(E type) const
	{
		return std::find(this->elements.begin(), this->elements.end(), type) != this->elements.end();
	}

	template<tz::enum_class E>
	bool EnumField<E>::contains(const EnumField<E>& field) const
	{
		for(E type : field.elements)
		{
			if(!this->contains(type))
			{
				return false;
			}
		}
		return true;
	}

	template<tz::enum_class E>
	std::size_t EnumField<E>::count() const
	{
		return this->elements.size();
	}

	template<tz::enum_class E>
	bool EnumField<E>::empty() const
	{
		return this->count() == 0;
	}

	template<tz::enum_class E>
	EnumField<E>& EnumField<E>::operator|=(E type)
	{
		{
			this->elements.push_back(type);
		}
		return *this;
	}

	template<tz::enum_class E>
	EnumField<E>& EnumField<E>::operator|=(const EnumField<E>& field)
	{
		for(E val : field)
		{
			*this |= val;
		}
		return *this;
	}

	template<tz::enum_class E>
	EnumField<E> EnumField<E>::operator|(E type) const
	{
		EnumField<E> cpy = *this;
		return cpy |= type;
	}

	template<tz::enum_class E>
	void EnumField<E>::remove(E type)
	{
		this->elements.erase(std::remove(this->elements.begin(), this->elements.end(), type), this->elements.end());
	}

	template<tz::enum_class E>
	auto EnumField<E>::begin() const
	{
		return this->elements.begin();
	}

	template<tz::enum_class E>
	auto EnumField<E>::begin()
	{
		return this->elements.begin();
	}

	template<tz::enum_class E>
	auto EnumField<E>::end() const
	{
		return this->elements.end();
	}

	template<tz::enum_class E>
	auto EnumField<E>::end()
	{
		return this->elements.end();
	}

	template<tz::enum_class E>
	const E& EnumField<E>::front() const
	{
		return this->elements.front();
	}

	template<tz::enum_class E>
	const E& EnumField<E>::back() const
	{
		return this->elements.back();
	}

	template<tz::enum_class E>
	EnumField<E>::operator E() const
	{
		using UnderlyingType = std::underlying_type_t<E>;
		if(this->elements.empty())
		{
			return E{};
		}
		auto e = static_cast<UnderlyingType>(this->elements.front());
		for(E ele : this->elements)
		{
			e |= static_cast<UnderlyingType>(ele);
		}
		return static_cast<E>(e);
	}

}
