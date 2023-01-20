#include "tz/core/tz_core.hpp"
#include "tz/core/macros.hpp"
#include "tz/core/debug.hpp"
#include <cstdio>

#include ImportedTextHeader(test_file, txt)

int main()
{
	tz::core::initialise();
	
	int answer;
	std::sscanf(ImportedTextData(test_file, txt).data(), "%d", &answer);
	tz::assert(answer == 42, "Imported text failed. Expected %d, got %d", 42, answer);

	tz::core::terminate();
}
