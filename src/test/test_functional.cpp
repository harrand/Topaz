
#define UNIT_TEST "Functional"
#include "lib/test_util.hpp"
#include "utility/functional.hpp"

void test()
{
    Functor increment = [](int x){return ++x;};
    int ret = increment(1);
    tz::assert::equal(ret, 2);
    std::vector<Runnable> tasks;
    tasks.emplace_back([&ret](){ret += 5;});
    tasks.emplace_back([&ret](){ret *= 2;});
    for(const auto& task : tasks)
        task();
    tz::assert::equal(ret, 14);
    int fib_a = 0;
    int fib_b = 1;
    Runnable fibonacci_step{[&](){int fib = fib_a + fib_b; fib_a = fib_b; fib_b = fib;}};
    Functor fibonacci = [&](std::size_t number)->int{for(std::size_t i = 0; i < number - 1; i++){fibonacci_step();}return fib_b;};
    // 40th fibonacci number is 102334155. will pass if runnable + functor combo can produce correct results.
    tz::assert::equal(fibonacci(40), 102334155);
}