module;
#include <format>
#include <string>
export module topaz.os:hardware_common;

export namespace tz::os
{
	struct monitor
	{

	};

	struct system
	{
		std::string computer_name;
		std::string cpu_name;
		unsigned int logical_processor_count;
		unsigned int page_size;
		unsigned long long physical_memory_kib;

		std::string to_string() const
		{
			return std::format(R"(
name: {}
cpu: {}
processor count: {}
page size: {}B
physical memory: {}MiB
			)",
			this->computer_name,
			this->cpu_name,
			this->logical_processor_count,
			this->page_size,
			this->physical_memory_kib / 1024u
			);
		}
	};
}
