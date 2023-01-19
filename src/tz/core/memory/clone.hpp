#ifndef TZ_MEMORY_CLONE_HPP
#define TZ_MEMORY_CLONE_HPP
#include <concepts>
#include <memory>
#include <type_traits>

namespace tz
{
	template<typename T>
	class unique_cloneable
	{
	public:
		[[nodiscard]] virtual std::unique_ptr<T> unique_clone() const = 0;
	};

	#define TZ_COPY_UNIQUE_CLONEABLE(I) virtual std::unique_ptr<I> unique_clone() const final \
	{ \
		using T = std::decay_t<decltype(*this)>; \
		static_assert(std::is_base_of_v<I, T>, "TZ_COPY_UNIQUE_CLONEABLE(I) must be invoked in a class field context such that the class T is a derived class of I."); \
		return std::unique_ptr<I>{static_cast<I*>(std::make_unique<T>(*this).release())}; \
	};

}


#endif // TZ_MEMORY_CLONE_HPP
