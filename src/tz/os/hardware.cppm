module;

#ifdef _WIN32
	#include "windows/winapi.hpp"
	#include <intrin.h>
#else
	// TODO: Linux Includes.
#endif
#include <string>
#include <format>
#include <vector>
#include <utility>

export module topaz.os:hardware;
import topaz.debug;

export namespace tz::os
{
	//						  Definitions						//
	//----------------------------------------------------------//
	struct monitor
	{
		std::string name;
		unsigned int resolutionw;
		unsigned int resolutionh;
	};

	using monitor_list = std::vector<monitor>;

	struct system
	{
		std::string computer_name;
		std::string cpu_name;
		unsigned int logical_processor_count;
		unsigned int page_size;
		unsigned long long physical_memory_kib;
		monitor_list monitors;

		std::string to_string() const
		{
			std::string monitors_section = "{";
			for(std::size_t i = 0; i < this->monitors.size(); i++)
			{
				const auto& mon = this->monitors[i];
				monitors_section += std::format("\n\t{}name: {}\n\tdimensions = {}x{}\n", i == 0 ? "primary monitor\n\t" : "", mon.name, mon.resolutionw, mon.resolutionh);
			}
			monitors_section += "}";

			return std::format(R"(
name: {}
cpu: {}
processor count: {}
page size: {}B
physical memory: {}MiB
monitors: {}
			)",
			this->computer_name,
			this->cpu_name,
			this->logical_processor_count,
			this->page_size,
			this->physical_memory_kib / 1024u,
			monitors_section
			);
		}
	};

	system get_system_windows();
	system get_system_linux();

	system get_system()
	{
		#ifdef _WIN32
			return get_system_windows();
		#elif defined(__linux__)
			return get_system_linux();
		#else
			return {};
		#endif
	}
}

namespace tz::os
{
	//	  				Windows Implementation					//
	//----------------------------------------------------------//
#ifdef _WIN32
	BOOL CALLBACK mon_enum_proc(HMONITOR hmon, [[maybe_unused]] HDC hdcmon, [[maybe_unused]] LPRECT lprcmon, LPARAM dwdata)
	{
		auto mons = reinterpret_cast<monitor_list*>(dwdata);
		MONITORINFOEXA minfo;
		minfo.cbSize = sizeof(MONITORINFOEXA);
		GetMonitorInfo(hmon, &minfo);
		mons->push_back
		({
			.name = minfo.szDevice,
			.resolutionw = static_cast<unsigned int>(minfo.rcMonitor.right - minfo.rcMonitor.left),
			.resolutionh = static_cast<unsigned int>(minfo.rcMonitor.bottom - minfo.rcMonitor.top)
		});
		if(hmon == MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY))
		{
			std::swap(mons->front(), mons->back());
		}
		return TRUE;
	}

	tz::os::system get_system_windows()
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

		EnumDisplayMonitors(nullptr, nullptr, mon_enum_proc, reinterpret_cast<LPARAM>(&ret.monitors));

		return ret;
	}

	//	  				Linux Implementation					//
	//----------------------------------------------------------//
#elif defined(__linux__)
	tz::os::system get_system_linux()
	{
		tz::debug::error("get_system() is not yet implemented on linux.");	
		return {};
	}
#else
		static_assert(false);
#endif
}
