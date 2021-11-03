#include "core/tz.hpp"
#include "gl/impl/backend/vk/tz_vulkan.hpp"
#include "gl/impl/backend/vk2/hardware/physical_device.hpp"
#include "gl/impl/backend/vk2/shader.hpp"
#include <string>
#include <fstream>

std::vector<char> get_empty_shader_spirv()
{
	const char* path = "./test/gl/vk2/shaders/empty.glsl.spv";
	std::ifstream fstr{path, std::ios::ate | std::ios::binary};
	if(!fstr.is_open())
	{
		tz_error("Could not read \"%s\", is this test running in the expected working directory?", path);
		return {};
	}
	auto file_size_bytes = static_cast<std::size_t>(fstr.tellg());
	fstr.seekg(0);
	std::vector<char> buffer;
	buffer.resize(file_size_bytes);
	fstr.read(buffer.data(), file_size_bytes);
	fstr.close();
	return buffer;
}

void basic_shader_module()
{
	using namespace tz::gl::vk2;

	PhysicalDevice pdev = get_all_devices().front();
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;
	LogicalDevice ldev{linfo};

	ShaderModuleInfo info;
	info.device = &ldev;
	info.code = get_empty_shader_spirv();
	info.type = ShaderType::Fragment;

	ShaderModule basic_module{info};
}

void basic_shader()
{
	using namespace tz::gl::vk2;

	PhysicalDevice pdev = get_all_devices().front();
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;
	LogicalDevice ldev{linfo};

	ShaderInfo sinfo;
	{
		ShaderModuleInfo info;
		info.device = &ldev;
		info.code = get_empty_shader_spirv();
		info.type = ShaderType::Fragment;

		sinfo.device = &ldev;
		sinfo.modules = {std::move(info)};
	}
	Shader shader{sinfo};

	auto native_data = shader.native_data();
	tz_assert(native_data.create_infos.length() == 1, "Shader.native_data() returned %zu elements, expected %d", native_data.create_infos.length(), 1);
}

int main()
{
	tz::GameInfo game{"vk_shader_test", tz::Version{1, 0, 0}};
	tz::initialise(game, tz::ApplicationType::Headless);
	tz::gl::vk2::initialise_headless(game, tz::ApplicationType::Headless);
	{
		basic_shader_module();
		basic_shader();
	}
	tz::gl::vk2::terminate();
	tz::terminate();
}
