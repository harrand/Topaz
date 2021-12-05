#include "gl/impl/backend/vk2/buffer.hpp"
#include "gl/impl/backend/vk2/hardware/physical_device.hpp"

void basic_buffers()
{
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	LogicalDevice ldev
	{{
		.physical_device = pdev,
	}};

	Buffer basic_vtx
	{{
		.device = &ldev,
		.size_bytes = sizeof(float),
		.usage = {BufferUsage::VertexBuffer},
		.residency = MemoryResidency::CPUPersistent
	}};

	{
		std::span<float> float_data = basic_vtx.map_as<float>();
		tz_assert(float_data.size() == 1 && float_data.size_bytes() == sizeof(float), "Buffer::map_as<T> returned invalid span");
		float_data.front() = 5.0f;
	}
	basic_vtx.unmap();
}

int main()
{
	tz::GameInfo game{"vk_buffer_test", tz::Version{1, 0, 0}, tz::info()};
	tz::initialise(game, tz::ApplicationType::Headless);
	tz::gl::vk2::initialise(game, tz::ApplicationType::Headless);
	{
		basic_buffers();
	}
	tz::gl::vk2::terminate();
	tz::terminate();
}
