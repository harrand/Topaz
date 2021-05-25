//
// Created by Harrand on 21/12/2019.
//

#ifndef TOPAZ_TEST_FRAMEWORK_HPP
#define TOPAZ_TEST_FRAMEWORK_HPP
#include <iostream>
#include <vector>
#include "core/assert.hpp"

namespace tz::test
{
	class Case
	{
	public:
		Case(): Case("Untitled"){}
		Case(const char* name): case_name(name), erroneous(false){}
		
		void expect(bool expression)
		{
			if(!expression)
				this->erroneous = true;
		}
		
		template<typename... Args>
		void expect(bool expression, Args&&... args)
		{
			if(!expression)
			{
				this->erroneous = true;
				(std::cerr << ... << args) << "\n";
			}
		}

		template<typename... Args>
		void expectf(bool expression, const char* fmt, Args&&... args)
		{
			if(!expression)
			{
				this->erroneous = true;
				std::fprintf(stderr, fmt, std::forward<Args>(args)...);
				std::fprintf(stderr, "\n");
			}
		}
		
		int result() const
		{
			return erroneous ? -1 : 0;
		}
		
		const char* name() const
		{
			return this->case_name;
		}
	private:
		const char* case_name;
		bool erroneous;
	};

#ifndef TZ_STRINGIFY
#define TZ_STRINGIFY(a) #a
#endif

#ifdef TZ_TEST_BEGIN
#undef TZ_TEST_BEGIN
#endif
#define TZ_TEST_BEGIN(TEST_NAME) \
tz::test::Case TEST_NAME() \
{ \
	tz::test::Case test_case(TZ_STRINGIFY(TEST_NAME));

#ifdef TZ_TEST_END
#undef TZ_TEST_END
#endif
#define TZ_TEST_END \
	return test_case; \
}

#ifdef topaz_expect
#undef topaz_expect
#endif
#define topaz_expect(EXPRESSION, ...) (test_case.expect(EXPRESSION, "Expectation failure: ", #EXPRESSION, "\nIn file ", __FILE__, " on line ", __LINE__, ":\n\t", __VA_ARGS__))

#ifdef topaz_expectf
#undef topaz_expectf
#endif
#define topaz_expectf(EXPRESSION, fmt, ...) (test_case.expectf(EXPRESSION, "Expectation failure: %s\nIn file %s on line %d:\n\t" fmt, #EXPRESSION, __FILE__, __LINE__, __VA_ARGS__))

#ifdef topaz_expect_assert
#undef topaz_expect_assert
#endif
#define topaz_expect_assert(SHOULD_ASSERT, ...) (test_case.expect(tz::debug::test::assert_failure() == SHOULD_ASSERT, "Expectation failure for assert in file ", __FILE__, " on line ", __LINE__, ":\n\t", __VA_ARGS__))

	class Unit
	{
	public:
		Unit() = default;
		~Unit()
		{
			if(this->result() >= 0)
			{
				std::cerr << "All tests successful (" << this->cases.size() << "/" << this->cases.size() << " passed)\n";
			}
			else
			{
				std::vector<const char*> fail_names;
				std::size_t num_succeeds = 0;
				for(const auto& c : this->cases)
				{
					if (c.result() == 0)
						num_succeeds++;
					else
						fail_names.push_back(c.name());
				}
				std::cerr << "Test failures detected: (" << num_succeeds << "/" << this->cases.size() << " passed)\n";
				std::cerr << "List of case failures:\n";
				for(const char* failure_name : fail_names)
				{
					std::cerr << "\t" << failure_name << "\n";
				}
				std::cerr << "==================================================================================================\n";
			}
		}
		
		void add(Case&& test_case)
		{
			this->cases.push_back(test_case);
		}
		
		int result()
		{
			int res = 0;
			for(const Case& test_case : this->cases)
			{
				if(test_case.result() != 0)
					res = -1;
			}
			return res;
		}
	private:
		std::vector<Case> cases;
	};

}

#endif //TOPAZ_TEST_FRAMEWORK_HPP