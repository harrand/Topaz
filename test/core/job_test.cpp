#include "tz/core/tz_core.hpp"
#include "tz/core/job/job.hpp"
#include "tz/core/debug.hpp"
#include "tz/core/profile.hpp"
#include <numeric>
#include <array>
#include <algorithm>

#define TESTFUNC_BEGIN(n) void n(){tz::assert(!tz::job_system().any_work_remaining(), "Old jobs still running when entering new test function");
#define TESTFUNC_END ;tz::job_system().block_all(); tz::assert(!tz::job_system().any_work_remaining(), "Jobs still running after blocking at the end of test function");}

TESTFUNC_BEGIN(basic_job)
	TZ_PROFZONE("basic job", 0xFF00AA00);
	int x = 1;
	auto j = tz::job_system().execute([&x]()
	{
		x += 5;
	});
	tz::job_system().block(j);
	tz::assert(tz::job_system().complete(j));
	tz::assert(!tz::job_system().any_work_remaining());
	tz::assert(x == 6, "Expected value to be %d but it is %d", 6, x);
TESTFUNC_END

TESTFUNC_BEGIN(job_reuse)
	TZ_PROFZONE("job reuse", 0xFF00AA00);
	auto j = tz::job_system().execute([](){});
	tz::job_system().block_all();
	tz::assert(tz::job_system().complete(j));
	auto k = tz::job_system().execute([](){});
	//tz::assert(j == k, "Old job is not being re-used.");
TESTFUNC_END

TESTFUNC_BEGIN(multi_job)
	constexpr std::size_t job_chunk_length = 32;
	constexpr std::size_t job_size = job_chunk_length * job_chunk_length;
	std::array<int, job_size> x;
	std::iota(x.begin(), x.end(), 0);
	std::array<int, job_size> y;
	std::iota(y.rbegin(), y.rend(), 0);
	
	std::array<int, job_size> z;
	std::fill_n(z.begin(), job_size, 0);

	for(std::size_t i = 0; i < job_size; i += (job_size/job_chunk_length))
	{
		tz::job_system().execute([begin = i, end = i + job_chunk_length, &x, &y, &z]()
		{
			for(std::size_t j = begin; j < end; j++)
			{
				z[j] = x[j] + y[j];
			}
		});
	}
	tz::job_system().block_all();

	tz::assert(std::all_of(z.begin(), z.end(), [job_size](int x){return x == job_size-1;}), "Multi job did not have expected output. Expected all elements of `z` to be %zu but %zu are", job_size, std::count_if(z.begin(), z.end(), [job_size](int x){return x == job_size-1;}));
TESTFUNC_END

int main()
{
	tz::core::initialise();

	for(std::size_t i = 0; i < 8; i++)
	{
		basic_job();
		job_reuse();
		multi_job();
	}

	tz::core::terminate();
	return 0;
}
