#include "tz/topaz.hpp"
#include "tz/os/file.hpp"

#include "tz/main.hpp"
int tz_main()
{
	std::string ret = tz_must(tz::os::read_file("./files/secret.txt"));
	tz_assert(ret == "42", "");
	return 0;
}