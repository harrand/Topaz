#include "render/device.hpp"
#include "core/debug/assert.hpp"
#include "gl/frame.hpp"
#include "gl/shader.hpp"
#include "gl/object.hpp"

namespace tz::render
{
	Device::Device(tz::gl::IFrame* frame, tz::gl::ShaderProgram* program, tz::gl::Object* object): frame(frame), program(program), object(object), ibo_id(std::nullopt), snippets(){}

	void Device::set_handle(std::size_t id)
	{
		this->ibo_id = id;
	}

	void Device::set_indices(tz::gl::IndexSnippetList indices)
	{
		this->snippets = indices;
	}

	void Device::render() const
	{
		if(!this->ibo_id.has_value())
			return;
		if(frame->operator!=(tz::gl::bound::frame()))
			frame->bind();
		program->bind();
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
	
	void Device::ensure_bound() const
	{
		topaz_assert(this->frame != nullptr, "tz::render::Device::ensure_bound(): There is no tz::gl::Frame attached!");
		if(this->frame->operator!=(tz::gl::bound::frame()))
			this->frame->bind();
		topaz_assert(this->program != nullptr, "tz::render::Device::ensure_bound(): There is no tz::gl::ShaderProgram attached!");
		if(this->program->operator!=(tz::gl::bound::shader_program()))
			this->program->bind();
	}
}