//
// Created by Harry on 13/10/2019.
//

#define UNIT_TEST "Memory"
#include "lib/test_util.hpp"

void test_memory_pool()
{
    MemoryPool<int> int_pool;
    tz::assert::that(int_pool.null());
    tz::assert::that(int_pool.empty());

    {
        // Pool starts at the address of temp, but is still of size 0.
        int temp = 5;
        int_pool = {&temp, 0};
        tz::assert::that(!int_pool.null());
        tz::assert::that(int_pool.empty());
    }

    std::array<int, 64> int_buffer{0};
    // Zero'd int buffer.
    int_pool = {int_buffer.data(), int_buffer.size()};
    tz::assert::that(!int_pool.null());
    tz::assert::that(!int_pool.empty());
    // Firstly, make sure int pool correctly recognises that all elements are 0
    auto size = int_pool.get_element_capacity();
    // Make sure the size is actually 64.
    tz::assert::that(size == 64);
    tz::assert::that(int_pool.get_byte_capacity() == 64 * sizeof(int));

    {
        std::size_t i = 0;
        for (int element : int_pool)
        {
            tz::assert::that(element == 0);
            i++;
        }
        // We should iterate exactly 64 times!
        tz::assert::that(i == size);
    }

    for(std::size_t i = 0; i < int_pool.get_element_capacity(); i++)
    {
        // Ensure all changes to the pool reflect in the corresponding element in the original buffer.
        int_pool[i] = static_cast<int>(i);
        tz::assert::that(int_buffer[i] == static_cast<int>(i));
    }

    int_pool.zero_all();
    for(int i : int_pool)
    {
        tz::assert::that(i == 0);
    }
}

void test_ampool()
{
    AutomaticMemoryPool<int> amp{8};
    tz::assert::that(amp.get_element_capacity() == 8);
    tz::assert::that(!amp.null());
    amp.zero_all();
    for(int i : amp)
    {
        tz::assert::that(i == 0);
    }
    amp[7] = 50;

    // Should be a deep-copy.
    AMPool<int> amp_copy = amp;
    tz::assert::that(amp_copy.get_element_capacity() == 8);
    tz::assert::that(amp_copy[7] == 50);
    amp_copy[7] = 800;
    tz::assert::that(amp_copy[7] == 800);
    tz::assert::that(amp[7] == 50);
}

void test_svmpool()
{
    std::tuple<int, float, double> buffer;
    SVMPool<int, float, double> pool{&buffer, 5, 1.0f, 18.0};
    tz::assert::that(pool.get_element_capacity() == 3);
    // Ensure values are correct.
    tz::assert::that(pool.get<0>() == pool.get<int>());
    tz::assert::that(pool.get<0>() == 5);

    tz::assert::that(pool.get<1>() == pool.get<float>());
    tz::assert::that(pool.get<1>() == 1.0f);

    tz::assert::that(pool.get<2>() == pool.get<double>());
    tz::assert::that(pool.get<2>() == 18.0);

    pool.get<2>() = 25.0;
    tz::assert::that(pool.get<double>() == 25.0);
    tz::assert::that(std::get<2>(buffer) == 25.0);
}

struct DVMFixture
{
public:
    static int& get_count()
    {
        static int count = 0;
        return count;
    }
    DVMFixture()
    {
        std::cout << "DVMFixture()";
        get_count()++;
    }
    DVMFixture(const DVMFixture& copy)
    {
        std::cout << "DVMFixture(const DVMFixture&)";
        get_count()++;
    }
    DVMFixture(DVMFixture&& move)
    {
        std::cout << "DVMFixture(DVMFixture&&)";
        get_count()++;
    }
    ~DVMFixture()
    {
        std::cout << "~DVMFixture()";
        get_count()--;
    }

    DVMFixture& operator=(const DVMFixture& copy)
    {
        std::cout << "DVMFixture copy assignment";
    }
};

void test_dvmpool()
{
    constexpr std::size_t buffer_size = 1024;
    char buffer[buffer_size];
    DVMPool pool{buffer, buffer_size};
    tz::assert::that(pool.empty());
    tz::assert::that(!pool.null());
    pool.push_back<int>(5);
    tz::assert::that(pool.get_type_at_index(0) == typeid(int));
    tz::assert::that(pool.at<int>(0) == 5);
    tz::assert::that(pool.get_size() == 1);
    pool.zero_all();
    tz::assert::that(pool.get_size() == 1);
    tz::assert::that(pool.at<int>(0) == 0);
    pool.push_back<std::string>("well met");
    tz::assert::that(pool.at<std::string>(1) == "well met");

    // Do we call destructors?
    tz::debug::print(DVMFixture::get_count());
    tz::assert::that(DVMFixture::get_count() == 0);
    pool.push_back<DVMFixture>({});
    tz::debug::print(DVMFixture::get_count());
    tz::assert::that(DVMFixture::get_count() == 1);
    pool.clear();
    tz::debug::print(DVMFixture::get_count());
    tz::assert::that(DVMFixture::get_count() == 0);
}

void test()
{
    test_memory_pool();
    test_ampool();
    test_svmpool();
    test_dvmpool();
}