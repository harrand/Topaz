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
	static_assert(tz::arithmetic<int>);
	static_assert(tz::arithmetic<float>);
	static_assert(tz::arithmetic<char>);
	static_assert(tz::arithmetic<bool>);
	static_assert(!tz::arithmetic<int*>);

	static_assert(tz::const_type<const int>);
	static_assert(!tz::const_type<int>);
	static_assert(!tz::const_type<const int*>);
	static_assert(tz::const_type<const int* const>);

	enum class EC{};
	enum E{};
	static_assert(!tz::enum_class<E>);
	static_assert(tz::enum_class<EC>);

	static_assert(tz::number<int>);
	static_assert(tz::number<float>);
	static_assert(!tz::number<int*>);
	static_assert(!tz::number<bool>);
	static_assert(!tz::number<char>);

	constexpr auto lambda0 = [](int x)constexpr->double{return x * 2;};
	static_assert(tz::function<decltype(lambda0), double, int>);
	static_assert(tz::function<decltype(fimpl0), double, int>);

	constexpr auto lambda1 = [](){};
	static_assert(tz::action<decltype(lambda1)>);
	static_assert(tz::action<decltype(fimpl1)>);
}
