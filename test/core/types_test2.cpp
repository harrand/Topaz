#include "tz/core/types.hpp"

constexpr void traits();

int main()
{
	traits();
	return 0;
}

double fimpl0(int){return 0.0;}
void fimpl1(){}

constexpr void traits()
{
	static_assert(tz::Arithmetic<int>);
	static_assert(tz::Arithmetic<float>);
	static_assert(tz::Arithmetic<char>);
	static_assert(tz::Arithmetic<bool>);
	static_assert(!tz::Arithmetic<int*>);

	static_assert(tz::ConstType<const int>);
	static_assert(!tz::ConstType<int>);
	static_assert(!tz::ConstType<const int*>);
	static_assert(tz::ConstType<const int* const>);

	enum class EC{};
	enum E{};
	static_assert(!tz::EnumClass<E>);
	static_assert(tz::EnumClass<EC>);

	static_assert(tz::Number<int>);
	static_assert(tz::Number<float>);
	static_assert(!tz::Number<int*>);
	static_assert(!tz::Number<bool>);
	static_assert(!tz::Number<char>);

	constexpr auto lambda0 = [](int x)constexpr->double{return x * 2;};
	static_assert(tz::Function<decltype(lambda0), double, int>);
	static_assert(tz::Function<decltype(fimpl0), double, int>);

	constexpr auto lambda1 = [](){};
	static_assert(tz::Action<decltype(lambda1)>);
	static_assert(tz::Action<decltype(fimpl1)>);
}
