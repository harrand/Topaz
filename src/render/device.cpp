#include "render/device.hpp"
#include "core/debug/assert.hpp"
#include "gl/frame.hpp"
#include "gl/shader.hpp"
#include "gl/object.hpp"

namespace tz::render
{
	Device::Device(tz::gl::IFrame* frame, tz::gl::ShaderProgram* program, tz::gl::Object* object): frame(frame), program(program), object(object), ibo_id(std::nullopt), snippets(), resource_buffers(){}

	void Device::set_frame(tz::gl::IFrame* frame)
	{
		this->frame = frame;
	}

	void Device::set_program(tz::gl::ShaderProgram* program)
	{
		this->program = program;
	}

	void Device::set_object(tz::gl::Object* object)
	{
		this->object = object;
		this->ibo_id = std::nullopt;
	}

	bool Device::contains_resource_buffer(const tz::gl::IBuffer* buffer) const
	{
		return std::find(this->resource_buffers.begin(), this->resource_buffers.end(), buffer) != this->resource_buffers.end();
	}

	void Device::add_resource_buffer(const tz::gl::IBuffer* buffer)
	{
		if(!this->contains_resource_buffer(buffer))
		{
			this->resource_buffers.push_back(buffer);
		}
	}

	void Device::remove_resource_buffer(const tz::gl::IBuffer* buffer)
	{
		if(this->contains_resource_buffer(buffer))
		{
			this->resource_buffers.erase(std::remove(this->resource_buffers.begin(), this->resource_buffers.end(), buffer));
		}
	}

	/*static*/ Device Device::null_device()
	{
		return {nullptr, nullptr, nullptr};
	}

	bool Device::is_null() const
	{
		return *this == Device::null_device();
	}

	void Device::set_handle(std::size_t id)
	{
		this->ibo_id = id;
		this->snippets = {};
	}

	void Device::set_handle(tz::gl::Object* object, std::size_t id)
	{
		this->object = object;
		this->set_handle(id);
	}

	void Device::set_indices(tz::gl::IndexSnippetList indices)
	{
		topaz_assert(this->ibo_id.has_value(), "tz::render::Device::set_indices(...): Cannot set indices without an IBO being provided beforehand.");
		#if TOPAZ_DEBUG
			tz::gl::IBO* ibo = this->object->get<tz::gl::BufferType::Index>(this->ibo_id.value());
			topaz_assert(ibo != nullptr, "tz::render::Device::set_indices(...): No valid IBO handle is set.");
			topaz_assert(this->sanity_check(indices, *ibo), "tz::render::Device::set_indices(...): Sanity-check failed! IndexSnippetList is malformed.");
		#endif
		this->snippets = indices;
	}

	void Device::render() const
	{
		topaz_assert(this->ready(), "tz::render::Device::render(): Device is not ready!");
		if(!this->ibo_id.has_value())
			return;
		if(frame->operator!=(tz::gl::bound::frame()))
			frame->bind();
		program->bind();
		for(const tz::gl::IBuffer* resource_buffer : this->resource_buffers)
		{
			resource_buffer->bind();
		}
		if (this->snippets.empty())
			this->object->render(this->ibo_id.value());
		else
		{
			// use MDI.
			this->object->multi_render(this->ibo_id.value(), this->snippets.get_command_list());
		}
	}

	void Device::clear() const
	{
		topaz_assert(this->frame != nullptr, "tz::render::Device::clear(): There is no tz::gl::Frame attached!");
		this->frame->bind();
		this->frame->clear();
	}

	bool Device::ready() const
	{
		// Firstly make sure frame, program and object are valid.
		if( this->frame == nullptr
		|| this->program == nullptr
		|| this->object == nullptr)
		{
			return false;
		}

		if(!this->frame->complete())
			return false;

		if(!this->program->usable())
			return false;

		return true;
	}

	bool Device::operator==(const tz::render::Device& rhs) const
	{
		return this->frame == rhs.frame
			&& this->program == rhs.program
			&& this->object == rhs.object;
	}
	
	void Device::ensure_bound() const
	{
		topaz_assert(this->frame != nullptr, "tz::render::Device::ensure_bound(): There is no tz::gl::Frame attached!");
		if(this->frame->operator!=(tz::gl::bound::frame()))
			this->frame->bind();
		topaz_assert(this->program != nullptr, "tz::render::Device::ensure_bound(): There is no tz::gl::ShaderProgram attached!");
		if(this->program->operator!=(tz::gl::bound::shader_program()))
			this->program->bind();
	}

	/*static*/ bool Device::sanity_check(const tz::gl::IndexSnippetList& indices, const tz::gl::IBO& ibo)
	{
		// Ensures that each snippet's offset is less than the number of indices.
		const std::size_t index_count = ibo.size() / sizeof(tz::gl::Index);
		const std::size_t sz = indices.size();
		for(std::size_t i = 0; i < sz; i++)
		{
			const tz::gl::IndexSnippet& cur_snippet = indices[i];
			if(cur_snippet.begin > index_count || cur_snippet.end > index_count)
			{
				return false;
			}
		}
		return true;
	}

}