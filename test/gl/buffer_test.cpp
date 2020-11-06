//
// Created by Harrand on 25/12/2019.
//

#include "test_framework.hpp"
#include "core/core.hpp"
#include "core/tz_glad/glad_context.hpp"
#include "gl/object.hpp"
#include "algo/static.hpp"

template<tz::gl::BufferType Type>
constexpr bool confirm()
{
	using BufferT = tz::gl::Buffer<Type>;
	return !tz::algo::copyable<BufferT>() && tz::algo::moveable<BufferT>();
}

tz::test::Case statics()
{
	tz::test::Case test_case("tz::gl::Buffer Static Tests");
	using namespace tz::gl;
	topaz_expect(test_case, confirm<BufferType::AtomicCounter>(), "tz::gl ACBO failed static tests");
	topaz_expect(test_case, confirm<BufferType::CopySource>(), "tz::gl CSBO failed static tests");
	topaz_expect(test_case, confirm<BufferType::CopyDestination>(), "tz::gl CDBO failed static tests");
	topaz_expect(test_case, confirm<BufferType::IndirectComputeDispatchCommand>(), "tz::gl DICBO failed static tests");
	topaz_expect(test_case, confirm<BufferType::IndirectCommandArgument>(), "tz::gl DIBO failed static tests");
	topaz_expect(test_case, confirm<BufferType::Index>(), "tz::gl IBO failed static tests");
	topaz_expect(test_case, confirm<BufferType::PixelReadTarget>(), "tz::gl PPBO failed static tests");
	topaz_expect(test_case, confirm<BufferType::TextureDataSource>(), "tz::gl PUBO failed static tests");
	topaz_expect(test_case, confirm<BufferType::QueryResult>(), "tz::gl QBO failed static tests");
	topaz_expect(test_case, confirm<BufferType::ShaderStorage>(), "tz::gl SSBO failed static tests");
	topaz_expect(test_case, confirm<BufferType::TextureData>(), "tz::gl TBO failed static tests");
	topaz_expect(test_case, confirm<BufferType::TransformFeedback>(), "tz::gl TFBO failed static tests");
	topaz_expect(test_case, confirm<BufferType::UniformStorage>(), "tz::gl UBO failed static tests");
	return test_case;
}

tz::test::Case binding()
{
	tz::test::Case test_case("tz::gl::Buffer Binding Tests");
	tz::gl::Object obj;
	std::size_t idx = obj.emplace_buffer<tz::gl::BufferType::Array>();
	tz::gl::VertexBuffer* buf = obj.get<tz::gl::BufferType::Array>(idx);
	obj.bind_child(idx);
	topaz_expect(test_case, *buf == tz::gl::bound::vertex_buffer(), "tz::gl::IBuffer bind failed to reflect in global state (global state handle = ", tz::gl::bound::vertex_buffer(), ")");
	buf->unbind();
	topaz_expect(test_case, *buf != tz::gl::bound::vertex_buffer(), "tz::gl::IBuffer unbind failed to reflect in global state (global state handle = ", tz::gl::bound::vertex_buffer(), ")");
	return test_case;
}

tz::test::Case mapping()
{
	tz::test::Case test_case("tz::gl::Buffer Mapping Tests");
	tz::gl::Object obj;
	std::size_t idx = obj.emplace_buffer<tz::gl::BufferType::Array>();
	tz::gl::IBuffer* buf = obj[idx];
	topaz_expect(test_case, buf != nullptr, "tz::gl::Object[", idx, "] gave nullptr. Something is horribly wrong.");
	topaz_expect(test_case, buf->valid(), "tz::gl::Buffer was wrongly considered invalid after construction");
	topaz_expect(test_case, glGetError() == 0, "glGetError() displayed an error!");
	topaz_expect(test_case, buf->empty(), "tz::gl::IBuffer constructed in object is not empty!");
	topaz_expect_assert(test_case, false, "tz::gl::IBuffer asserted at the wrong time (Probably from Buffer<T>::size())...");
	constexpr std::size_t amt = 5;

	// Now lets allocate some data in it.
	buf->resize(sizeof(float) * amt);
	topaz_expect(test_case, !buf->is_mapped(), "tz::gl::IBuffer thinks it's mapped when it really shouldn't be...");
	topaz_expect(test_case, !buf->empty(), "tz::gl::IBuffer still thought it was empty after a resize.");
	{
		tz::mem::Block mapping = buf->map();
		topaz_expect(test_case, buf->is_mapped(), "tz::gl::IBuffer doesn't think it's mapped when it definitely should be...");
		topaz_expect(test_case, mapping.size() == (sizeof(float) * amt), "tz::gl::IBuffer mapping had unexpected size. Expected ", sizeof(float)*amt, ", but got ", mapping.size());
		topaz_expect_assert(test_case, false, "Unexpected assert invoked while testing tz::gl::Buffer Mapping. There are several possible causes -- Consider debugging.");
		buf->unmap();
	}

	constexpr float test_val = 862.123f;
	// Let's map directly to a pool and do an edit.
	{
		tz::mem::UniformPool<float> floats = buf->map_uniform<float>();
		floats.set(0, test_val);
		// Should definitely have capacity of 5.
		topaz_expect(test_case, floats.capacity() == 5, "Uniform float pool had unexpected capacity. Expected ", 5, " but got ", floats.capacity());
		buf->unmap();
	}
	// First thing in the mapped block should now be a float with the value of test_val.
	// Because a UniformPool amends the type-system (and float is not a struct with const members), we can type-pun it without issue.
	// Let's re-map it and check.
	{
		tz::mem::Block blk = buf->map();
		float first = *reinterpret_cast<float*>(blk.begin);
		topaz_expect(test_case, first == (test_val), "tz::gl::Buffer UniformPool mapping did not reflect in the VRAM data store. Expected value, ", test_val, ", but got ", first);
		buf->unmap();
	}
	return test_case;
}

tz::test::Case terminality()
{
	tz::test::Case test_case("tz::gl Buffer Terminality Tests");
	tz::gl::Object o;
	{
		std::size_t idx = o.emplace_buffer<tz::gl::BufferType::Array>();
		tz::gl::VBO* vbo = o.get<tz::gl::BufferType::Array>(idx);
		topaz_assert_clear();
		// Definitely shouldn't be terminal for now.
		topaz_expect(test_case, !vbo->is_terminal(), "tz::gl::Buffer wrongly considers itself to be terminal.");
		vbo->resize(1);
		// Mapping it normally certainly shouldn't make it terminal.
		topaz_expect(test_case, !vbo->is_mapped(), "tz::gl::Buffers wrongly considers itself to be mapped...");
		vbo->map();
		topaz_expect(test_case, vbo->is_mapped(), "tz::gl::Buffer wrongly considers itself to be unmapped...");
		topaz_expect(test_case, !vbo->is_terminal(), "tz::gl::Buffer wrongly considers itself to be terminal after non-terminal mapping.");
		vbo->unmap();
		// Let's resize this terminally!
		vbo->terminal_resize(1024);
		topaz_expect(test_case, vbo->is_terminal(), "tz::gl::Buffer failed to realise that it had become terminal after a terminal resize.");
		topaz_expect(test_case, vbo->size() == 1024, "tz::gl::Buffer had unexpected size. Expected ", 1024, " but got ", vbo->size());
		vbo->map();
		// The mapping is valid during render calls etc... There is no good way of testing for this however, as we can't test for the presence of UB without using UBsan which is likely not to detect this anyway...
		vbo->unmap();
		topaz_expect_assert(test_case, false, "tz::gl::Buffer unexpectedly asserted during terminality tests...");
	}

	// Have an existing buffer with some data, make it terminal and then ensure data hasn't been screwed up

	{
		std::size_t idx = o.emplace_buffer<tz::gl::BufferType::Array>();
		tz::gl::VBO* vbo = o.get<tz::gl::BufferType::Array>(idx);

		// It will store {0, 1, 2, 3}
		vbo->resize(sizeof(int) * 4);
		int data[] = {0, 1, 2, 3};
		vbo->send(data);

		auto ensure = [data, vbo]()
		{
			tz::mem::Block blk = vbo->map();
			for(std::size_t i = 0; i < 4; i++)
			{
				auto cur = static_cast<int*>(blk.begin) + i;
				if(*cur != data[i])
				{
					vbo->unmap();
					return false;
				}
			}
			vbo->unmap();
			return true;
		};
		// Ensure should be true before and after making it terminal.
		topaz_expect(test_case, ensure(), "Ensure failed prior to making the buffer terminal.");
		vbo->make_terminal();
		topaz_expect(test_case, ensure(), "Ensure failed after making the buffer terminal. Did it trash the data?");
	}
	return test_case;
}

tz::test::Case retrieval()
{
	tz::test::Case test_case("tz::gl::Buffer General Retrieval Tests");
	tz::gl::Object o;
	std::size_t idx = o.emplace_buffer<tz::gl::BufferType::Array>();
	tz::gl::VBO* vbo = o.get<tz::gl::BufferType::Array>(idx);
	{
		// Let's work with floats.
		constexpr std::size_t amt = 8;
		constexpr std::size_t sz = sizeof(float) * amt;
		vbo->resize(sz);
		topaz_expect_assert(test_case, false, "tz::gl::Buffer unexpectedly asserted.");
		// Map it and write some data into it.
		{
			tz::mem::UniformPool<float> pool = vbo->map_uniform<float>();
			for(std::size_t i = 0; i < amt; i++)
				pool.set(i, 0.0f + i);
			// expect data-store to be: {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f}
			vbo->unmap();
		}

		// Firstly we'll check with no offset and a smaller size.
		{
			float data[amt - 4];
			// we'll query the first 4 floats.
			vbo->retrieve(0, 4 * sizeof(float), data);
			for(std::size_t i = 0; i < 4; i++)
				topaz_expect(test_case, data[i] == (0.0f + i), "tz::gl::Buffer retrieval of a subset of the data store yielded incorrect value. Expected ", (0.0f + i), ", got ", data[i]);
		}

		// Then we'll check with an offset for the last 2 floats.
		{
			float data[2];
			vbo->retrieve(6 * sizeof(float), 2 * sizeof(float), data);
			for(std::size_t i = 0; i < 2; i++)
				topaz_expect(test_case, data[i] == (6.0f + i), "tz::gl::Buffer retrieval of an offsetted subset of the data yielded incorrect value. Expected ", (6.0f + i), ", got ", data[i]);
		}
	}
	return test_case;
}

tz::test::Case nonterminal_retrieval()
{
	tz::test::Case test_case("tz::gl::Buffer Non-terminal Retrieval Tests");
	tz::gl::Object o;
	std::size_t idx = o.emplace_buffer<tz::gl::BufferType::Array>();
	tz::gl::VBO* vbo = o.get<tz::gl::BufferType::Array>(idx);
	{
		// Let's work with floats.
		constexpr std::size_t amt = 8;
		constexpr std::size_t sz = sizeof(float) * amt;
		vbo->resize(sz);
		topaz_expect_assert(test_case, false, "tz::gl::Buffer unexpectedly asserted.");
		// Map it and write some data into it.
		{
			tz::mem::UniformPool<float> pool = vbo->map_uniform<float>();
			for(std::size_t i = 0; i < amt; i++)
				pool.set(i, 0.0f + i);
			// expect data-store to be: {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f}
			// non-terminal buffers absolutely need to be unmapped.
			vbo->unmap();
		}
		float fblk[amt];
		vbo->retrieve_all(fblk);
		for(std::size_t i = 0; i < amt; i++)
		{
			topaz_expect(test_case, fblk[i] == (0.0f + i), "tz::gl::Buffer Retrieval Element was incorrect (Terminal). Expected ", (0.0f + i), ", got ", fblk[i]);
		}
		topaz_expect_assert(test_case, false, "tz::gl::Buffer Terminal Retrieval yielded unexpected assertion.");
	}
	return test_case;
}

tz::test::Case terminal_retrieval()
{
	// This is functionally identical to nonterminal_retrieval except that we don't unmap before retrieval (to test that edge-case).
	tz::test::Case test_case("tz::gl::Buffer Terminal Retrieval Tests");
	tz::gl::Object o;
	std::size_t idx = o.emplace_buffer<tz::gl::BufferType::Array>();
	tz::gl::VBO* vbo = o.get<tz::gl::BufferType::Array>(idx);
	{
		// Let's work with floats.
		constexpr std::size_t amt = 8;
		constexpr std::size_t sz = sizeof(float) * amt;
		vbo->terminal_resize(sz);
		topaz_expect_assert(test_case, false, "tz::gl::Buffer unexpectedly asserted.");
		// Map it and write some data into it.
		{
			tz::mem::UniformPool<float> pool = vbo->map_uniform<float>();
			for(std::size_t i = 0; i < amt; i++)
				pool.set(i, 0.0f + i);
			// expect data-store to be: {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f}
			// don't unmap as terminal buffers shouldn't require it.
			//vbo->unmap();
		}
		float fblk[amt];
		vbo->retrieve_all(fblk);
		for(std::size_t i = 0; i < amt; i++)
		{
			topaz_expect(test_case, fblk[i] == (0.0f + i), "tz::gl::Buffer Retrieval Element was incorrect (Terminal). Expected ", (0.0f + i), ", got ", fblk[i]);
		}
		topaz_expect_assert(test_case, false, "tz::gl::Buffer Terminal Retrieval yielded unexpected assertion.");
	}
	return test_case;
}

tz::test::Case sending()
{
	tz::test::Case test_case("tz::gl::Buffer General Sending Tests");
	tz::gl::Object o;
	std::size_t idx = o.emplace_buffer<tz::gl::BufferType::Array>();
	tz::gl::VBO* vbo = o.get<tz::gl::BufferType::Array>(idx);
	{
		// Let's try and send some ints.
		constexpr std::size_t amt = 3;
		constexpr std::size_t sz = sizeof(int) * amt;
		vbo->resize(sz);
		
		// Try sending via void*.
		{
			int data[amt] = {0, 1, 2};
			vbo->send(data);
			int* recvdata = reinterpret_cast<int*>(vbo->map().begin);
			for(std::size_t i = 0; i < amt; i++)
				topaz_expect(test_case, data[i] == recvdata[i], "tz::gl::Buffer::send(void*) seemed to fail to send data correctly. Expected value ", data[i], ", but got value ", recvdata[i]);
			vbo->unmap();
		}

		// Try sending via offset and block.
		{
			int data = 3;
			tz::mem::Block blk{&data, 1};
			const int offset_elements = (vbo->size() / sizeof(int)) - 1;
			const int offset_bytes = vbo->size() - sizeof(int);
			vbo->send(offset_bytes, blk);
			int* recvdata = reinterpret_cast<int*>(vbo->map().begin);

			topaz_expect(test_case, data == recvdata[offset_elements], "tz::gl::Buffer::send(", offset_bytes, ", tz::mem::Block (", blk.size(), ")): seemed to fail to send data correctly. Expected value ", data, ", but got value ", recvdata[offset_elements], " (offset_elements: ", offset_elements, ", offset_bytes = ", offset_bytes, ")");
			vbo->unmap();
		}
		
		// Try sending via range.
		{
			std::vector<int> data = {7, 8, 9};
			vbo->send_range(data.begin(), data.end());
			int* recvdata = reinterpret_cast<int*>(vbo->map().begin);
			for(std::size_t i = 0; i < amt; i++)
				topaz_expect(test_case, data[i] == recvdata[i], "tz::gl::Buffer::send_range(...): seemed to fail to send data correctly. Expected value ", data[i], ", but got value ", recvdata[i]);
			vbo->unmap();
		}
		
	}

	return test_case;
}

int main()
{
	tz::test::Unit buffer;

	// We require topaz to be initialised.
	{
		tz::core::initialise("Buffer Tests", tz::core::invisible_tag);

		buffer.add(statics());
		buffer.add(binding());
		buffer.add(mapping());
		buffer.add(terminality());
		buffer.add(retrieval());
		buffer.add(nonterminal_retrieval());
		buffer.add(terminal_retrieval());
		buffer.add(sending());

		tz::core::terminate();
	}
	return buffer.result();
}