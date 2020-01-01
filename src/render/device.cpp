#include "render/device.hpp"
#include "core/debug/assert.hpp"
#include "gl/frame.hpp"
#include "gl/shader.hpp"
#include "gl/object.hpp"

namespace tz::render
{
	Device::Device(tz::gl::IFrame* frame, tz::gl::ShaderProgram* program, tz::gl::Object* object): frame(frame), program(program), object(object), index_ids(){}

	void Device::specify_handles(std::initializer_list<std::size_t> ids)
	{
		for(std::size_t idx : ids)
			index_ids.push_back(idx);
	}

	void Device::render() const
	{
		//topaz_assert(!this->index_ids.empty(), "tz::render::Device::render(): No indices to render.");
		if(frame->operator!=(tz::gl::bound::frame()))
			frame->bind();
		program->bind();
		object->render(this->index_ids.begin(), this->index_ids.end());
	}
}