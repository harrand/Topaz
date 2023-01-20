#include "tz/core/debug.hpp"
#include "tz/core/memory/clone.hpp"

class A1 : public tz::unique_cloneable<A1>
{
public:
	virtual int foo() const = 0;
};

class A2 : public A1
{
public:
	A2() = default;
	virtual int foo() const final{return 1;}
	TZ_COPY_UNIQUE_CLONEABLE(A1);
};

void copy_unique_clone()
{
	A2 x;
	auto a1_clone = x.unique_clone();
	tz::assert(x.foo() == a1_clone->foo());
}

int main()
{
	copy_unique_clone();
}
