#ifndef TOPAZ_CORE_OPAQUE_HANDLE_HPP
#define TOPAZ_CORE_OPAQUE_HANDLE_HPP
#include <cstddef>

namespace tz
{
	enum class HandleValue : std::size_t{};
	using HandleValueUnderlying = std::underlying_type_t<HandleValue>;
	struct nullhand_t{};
	constexpr nullhand_t nullhand;

	template<typename T>
	class Handle
	{
	public:
		Handle(HandleValue value):
		value(value){}

		Handle(nullhand_t):
		value(static_cast<HandleValue>(std::numeric_limits<std::size_t>::max())){}

		explicit operator HandleValue() const
		{
			return this->value;
		}

		Handle& operator=(HandleValue value)
		{
			this->value = value;
			return *this;
		}
		bool operator==(nullhand_t) const
		{
			return this->value == static_cast<HandleValue>(0);
		}
		bool operator==(const Handle<T>& rhs) const = default;
	private:
		HandleValue value;
	};
}

namespace std
{
	template<typename T>
	struct hash<tz::Handle<T>>
	{
		std::size_t operator()(const tz::Handle<T>& handle) const
		{
			return std::hash<std::size_t>{}(static_cast<std::size_t>(static_cast<tz::HandleValue>(handle)));
		}
	};
}

#endif //TOPAZ_CORE_OPAQUE_HANDLE_HPP