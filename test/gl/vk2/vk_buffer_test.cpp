#include "gl/impl/backend/vk2/buffer.hpp"
#include "gl/impl/backend/vk2/gpu_mem.hpp"
#include "gl/impl/backend/vk2/hardware/physical_device.hpp"
#include "gl/impl/backend/vk2/command.hpp"
#include "gl/impl/backend/vk2/fence.hpp"

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

void staging_buffer()
{
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	LogicalDevice ldev
	{{
		.physical_device = pdev,
	}};

	hardware::Queue* queue = ldev.get_hardware_queue
	({
		.field = {QueueFamilyType::Graphics}
	});

	CommandPool cpool
	{{
		.queue = queue
	}};
	CommandPool::AllocationResult cres = cpool.allocate_buffers
	({
		.buffer_count = 1
	});
	CommandBuffer& cbuf = cres.buffers.front();

	// Create a staging buffer and a normal buffer.
	constexpr std::size_t float_count = 3;
	Buffer staging
	{{
		.device = &ldev,
		.size_bytes = sizeof(float) * float_count,
		.usage = {BufferUsage::TransferSource},
		.residency = MemoryResidency::CPU
	}};
	Buffer vtx_buf
	{{
		.device = &ldev,
		.size_bytes = sizeof(float) * float_count,
		.usage = {BufferUsage::VertexBuffer, BufferUsage::TransferDestination},
		.residency = MemoryResidency::GPU
	}};
	// Put {1.0f, 2.0f, 3.0f, ...} into the staging buffer.
	{
		std::span<float> staging_data = staging.map_as<float>();
		for(float i = 0.0f; i < staging_data.size(); i += 1.0f)
		{
			staging_data[static_cast<std::span<float>::size_type>(i)] = i;
		}
		staging.unmap();
	}
	// Record a buffer copy.
	{
		CommandBufferRecording record = cbuf.record();
		record.buffer_copy_buffer
		({
			.src = &staging,
			.dst = &vtx_buf
		});
	}
	// Submit the work. Use a fence so we can wait till its done.
	Fence fence
	{{
		.device = &ldev,
	}};

	queue->submit
	({
		.command_buffers = {&cbuf},
		.waits = {},
		.signal_semaphores = {},
		.execution_complete_fence = &fence
	});

	fence.wait_until_signalled();
}

int main()
{
	tz::GameInfo game{"vk_buffer_test", tz::Version{1, 0, 0}, tz::info()};
	tz::initialise(game, tz::ApplicationType::Headless);
	tz::gl::vk2::initialise(game, tz::ApplicationType::Headless);
	{
		basic_buffers();
		staging_buffer();
	}
	tz::gl::vk2::terminate();
	tz::terminate();
}
