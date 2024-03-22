module;

#ifdef _WIN32
	#include "windows/winapi.hpp"
	#include <intrin.h>
	#include <string>
	#include <format>
#else
	// TODO: Linux Includes.
#endif

export module topaz.os:hardware;

import topaz.debug;

export namespace tz::os
{
	//						  Definitions						//
	//----------------------------------------------------------//
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

	//	  				Windows Implementation					//
	//----------------------------------------------------------//
	#ifdef _WIN32
	system get_system()
	{
		system ret;

		// system info contains one or two useful bits.
		SYSTEM_INFO internal_sysinfo;
		GetSystemInfo(&internal_sysinfo);

		// computer name is easy enough.
		std::string computer_name;
		computer_name.resize(MAX_COMPUTERNAME_LENGTH + 1);
		DWORD computer_name_length;
		GetComputerNameA(computer_name.data(), &computer_name_length);
		computer_name.resize(computer_name_length);

		// cpu name is not fun. requires windows-specific cpuid intrinsics.
		int cpu_info[4] = {-1};
		std::string cpu_name;
		cpu_name.resize(0x40);

		memset(cpu_name.data(), 0, cpu_name.size());

		__cpuid(cpu_info, 0x80000002);
		memcpy(cpu_name.data(), cpu_info, sizeof(cpu_info));

		__cpuid(cpu_info, 0x80000003);
		memcpy(cpu_name.data() + 16, cpu_info, sizeof(cpu_info));

		__cpuid(cpu_info, 0x80000004);
		memcpy(cpu_name.data() + 32, cpu_info, sizeof(cpu_info));

		ret.computer_name = computer_name;
		ret.cpu_name = cpu_name;
		ret.logical_processor_count = internal_sysinfo.dwNumberOfProcessors;
		ret.page_size = internal_sysinfo.dwPageSize;
		GetPhysicallyInstalledSystemMemory(&ret.physical_memory_kib);
		return ret;
	}

	//	  				Linux Implementation					//
	//----------------------------------------------------------//
	#elif defined(__linux__)
	system get_system()
	{
		tz::debug::error("get_system() is not yet implemented on linux.");	
		return {};
	}
	#else
		static_assert(false);
	#endif
}
