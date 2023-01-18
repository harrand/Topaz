#include "tz/core/engine_info.hpp"

namespace tz
{
	std::string engine_info::to_string() const
	{
		const char* render_api;
		switch(this->renderer)
		{
			case render_api::opengl:
				render_api = "OpenGL";
			break;
			case render_api::vulkan:
				render_api = "Vulkan";
			break;
			default:
				hdk::error("Unrecognised graphics API. A serious error has occurred. Please submit a bug report");
				render_api = "Unknown";
			break;
		}
		const char* build_config;
		switch(this->build)
		{
			case build_config::debug:
				build_config = "Debug";
			break;
			case build_config::profile:
				build_config = "Profile";
			break;
			case build_config::release:
				build_config = "Release";
			break;
			default:
				hdk::error("Unrecognised build config. A serious error has occurred. Please submit a bug report");
				build_config = "Unknown";
			break;
		}
		std::string result = "Topaz v";
		result += this->version.to_string();
		result += " ";
		result += render_api;
		result += " ";
		result += build_config;
		return result;
	}

}
