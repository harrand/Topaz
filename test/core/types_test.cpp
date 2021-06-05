#include "core/tz.hpp"
#include "core/types.hpp"
#include <numbers>
#include <cmath>
#include <iostream>

void foo(){}

void print_hello()
{
    std::cout << "hello world!\n";
}

void print_something(const char* msg)
{
    std::cout << msg << "\n";
}

void sanity_check()
{
    auto expect_num = []<typename T>(T&& t, bool should_be) constexpr
    {
        #ifndef _MSC_VER
            // No MSVC support for 'requires' expression https://en.cppreference.com/w/cpp/compiler_support
            constexpr bool satisfies = requires{requires tz::Number<T>;};
            tz_assert(satisfies == should_be, "A type did not satisfy a sanity check (tz::Number<T>)");
        #endif
    };
    struct NotANumber
    {
        int x = 5;
    };
    expect_num(5, true);
    expect_num(12.0f, true);
    expect_num('c', false);
    expect_num("this is a char array", false);
    expect_num(NotANumber{5}, false);
    expect_num(NotANumber{6}.x, true); 
    expect_num(sanity_check, false);
    expect_num(expect_num, false);
    expect_num(bool{true}, false);
    expect_num(9 * 8 * 7 * std::sin(std::numbers::pi / 2.0f), true);

    auto expect_void_action = []<typename F>(F&& f, bool should_be) constexpr
    {
        constexpr bool satisfies = requires{requires tz::Action<F>;};
        tz_assert(satisfies == should_be, "A type did not satisfy a sanity check (tz::Action<F> aka 'void()')");
    };

    struct MyFunctor
    {
        void operator()(){}
    };

    expect_void_action(foo, true);
    expect_void_action(print_hello, true);
    expect_void_action(print_something, false);
    expect_void_action(5, false);
    expect_void_action(MyFunctor{}, true);

    auto expect_int_func_taking_two_ints = []<typename F>(F&& f, bool should_be) constexpr
    {
        constexpr bool satisfies = requires{requires tz::Function<F, int, int, int>;};
        tz_assert(satisfies == should_be, "A type did not satisfy a sanity check (tz::Function<F, int, int, int> aka 'int(int, int)')");
    };
    struct MyAdderFunctor
    {
        int operator()(int a, int b){return a + b;}
    };
    auto adder_lambda = [](int a, int b)->int{return a + b;};
    auto number_adder_lambda = [](tz::Number auto a, tz::Number auto b)->tz::Number auto{return a + b;};
    expect_int_func_taking_two_ints(MyAdderFunctor{}, true);
    expect_int_func_taking_two_ints(foo, false);
    expect_int_func_taking_two_ints(print_something, false);
    expect_int_func_taking_two_ints(adder_lambda, true);
    expect_int_func_taking_two_ints(number_adder_lambda, true);
}

void numeric()
{
    auto test = []<tz::Number T>(T t)
    {
        T initial = t;
        t *= 2;
        t -= initial;
        tz_assert(initial == t, "Multiplied by two and then subtracted by original did not yield original.");
    };
    test(5);
    test(16u);
    test(2893645847ull);
    test(0.5f);
    test(1.11);
    test(-12345);
}

tz::Number auto multiply_em(tz::Number auto num1, tz::Number auto num2)
{
    return num1 * num2;
}

int double_it(int x)
{
    return x + x;
}

template<tz::Action Func>
void doit(Func f)
{
    f();
}

template<typename... Args, tz::Action<Args...> Func>
void doit_withparams(Func f, Args&&... args)
{
    f(std::forward<Args>(args)...);
}

template<typename Return, typename... Args, tz::Function<Return, Args...> Func>
Return doit_witheverything(Func f, Args&&... args)
{
    return f(std::forward<Args>(args)...);
}

void functional_action()
{
    // Parameterless tz::Action
    int x = 1;
    auto dubs = [&x](){x *= 2;};
    doit(dubs);
    tz_assert(x == 2, "");

    // Parameters tz::Action
    auto muls = [&x](int mul){x *= mul;};
    doit_withparams(muls, 5);
    tz_assert(x == 10, "");

    // Parameters & Return tz::Function
    auto give_double = [](int x){return x*2;};
    int res = doit_witheverything<int>(give_double, 10);
    tz_assert(res == 20, "");
}

int main()
{
    sanity_check();
	numeric();
    functional_action();
}