#include "tz/gl/impl/backend/vk2/buffer.hpp"
#include "tz/gl/impl/backend/vk2/gpu_mem.hpp"
#include "tz/gl/impl/backend/vk2/hardware/physical_device.hpp"
#include "tz/gl/impl/backend/vk2/command.hpp"
#include "tz/gl/impl/backend/vk2/fence.hpp"
#include "tz/gl/impl/backend/vk2/semaphore.hpp"

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

void buffer_transfers()
{
	// Write data into a staging buffer. Use a command buffer to transfer the staging buffer data into a GPU buffer. After that, transfer that back into another CPU buffer and ensure the resultant data is the same.
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
		.buffer_count = 2
	});
	CommandBuffer& cbuf = cres.buffers[0];
	CommandBuffer& cbuf2 = cres.buffers[1];

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
		.usage = {BufferUsage::VertexBuffer, BufferUsage::TransferDestination, BufferUsage::TransferSource},
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
	// Submit the first transfer. Signal a semaphore when it's done.
	BinarySemaphore sem{ldev};

	queue->submit
	({
		.command_buffers = {&cbuf},
		.waits = {},
		.signal_semaphores = {&sem},
	});

	Buffer final_cpu_buf
	{{
		.device = &ldev,
		.size_bytes = sizeof(float) * float_count,
		.usage = {BufferUsage::TransferDestination},
		.residency = MemoryResidency::CPU
	}};

	{
		// Record the second transfer.
		CommandBufferRecording record = cbuf2.record();
		record.buffer_copy_buffer
		({
			.src = &vtx_buf,
			.dst = &final_cpu_buf
		});
	}
	// Submit the work, wait on a fence to make sure we only check once it's done.
	Fence fence
	{{
		.device = &ldev
	}};
	queue->submit
	({
		.command_buffers = {&cbuf2},
		.waits = 
		{
			hardware::Queue::SubmitInfo::WaitInfo
			{
				.wait_semaphore = &sem,
				.wait_stage = PipelineStage::TransferCommands
			}
		},
		.signal_semaphores = {},
		.execution_complete_fence = &fence
	});

	fence.wait_until_signalled();
	// Now check the data is correct.
	{
		std::span<float> output_data = final_cpu_buf.map_as<float>();
		
		for(std::size_t i = 0; i < output_data.size(); i++)
		{
			tz_assert(output_data[i] == static_cast<float>(i), "Output data was incorrect. Expected %g, got %g", static_cast<float>(i), output_data[i]);
		}

		final_cpu_buf.unmap();
	}
}

int main()
{
	tz::initialise
	({
		.name = "vk_buffer_test",
		.flags = {tz::ApplicationFlag::HiddenWindow},
	});
	{
		basic_buffers();
		buffer_transfers();
	}
	tz::terminate();
}
