//
// Created by Harry on 04/05/2019.
//

#include "generic_buffer.hpp"

namespace tz::platform
{
	OGLBufferUsage::OGLBufferUsage(OGLBufferFrequency frequency, OGLBufferNature nature) : frequency(frequency), nature(nature) {}

	OGLBufferUsage::OGLBufferUsage(const GLenum &usage) : frequency(OGLBufferFrequency::STATIC), nature(OGLBufferNature::DRAW)
	{
		switch (usage)
		{
			default:
			case GL_STREAM_DRAW:
				this->frequency = OGLBufferFrequency::STREAM;
				this->nature = OGLBufferNature::DRAW;
				break;
			case GL_STREAM_READ:
				this->frequency = OGLBufferFrequency::STREAM;
				this->nature = OGLBufferNature::READ;
				break;
			case GL_STREAM_COPY:
				this->frequency = OGLBufferFrequency::STREAM;
				this->nature = OGLBufferNature::COPY;
				break;
			case GL_STATIC_DRAW:
				this->frequency = OGLBufferFrequency::STATIC;
				this->nature = OGLBufferNature::DRAW;
				break;
			case GL_STATIC_READ:
				this->frequency = OGLBufferFrequency::STATIC;
				this->nature = OGLBufferNature::READ;
				break;
			case GL_STATIC_COPY:
				this->frequency = OGLBufferFrequency::STATIC;
				this->nature = OGLBufferNature::COPY;
				break;
			case GL_DYNAMIC_DRAW:
				this->frequency = OGLBufferFrequency::DYNAMIC;
				this->nature = OGLBufferNature::DRAW;
				break;
			case GL_DYNAMIC_READ:
				this->frequency = OGLBufferFrequency::DYNAMIC;
				this->nature = OGLBufferNature::READ;
				break;
			case GL_DYNAMIC_COPY:
				this->frequency = OGLBufferFrequency::DYNAMIC;
				this->nature = OGLBufferNature::COPY;
				break;
		}
	}

	GLenum OGLBufferUsage::operator()() const
	{
		// Nested switches, basically just multiplexing
		using namespace tz::platform;
		switch (this->frequency)
		{
			case OGLBufferFrequency::STREAM:
				switch (this->nature)
				{
					case OGLBufferNature::DRAW:
						return GL_STREAM_DRAW;
					case OGLBufferNature::READ:
						return GL_STREAM_READ;
					case OGLBufferNature::COPY:
						return GL_STREAM_COPY;
				}
			case OGLBufferFrequency::STATIC:
				switch (this->nature)
				{
					case OGLBufferNature::DRAW:
						return GL_STATIC_DRAW;
					case OGLBufferNature::READ:
						return GL_STATIC_READ;
					case OGLBufferNature::COPY:
						return GL_STATIC_COPY;
				}
			case OGLBufferFrequency::DYNAMIC:
				switch (this->nature)
				{
					case OGLBufferNature::DRAW:
						return GL_DYNAMIC_DRAW;
					case OGLBufferNature::READ:
						return GL_DYNAMIC_READ;
					case OGLBufferNature::COPY:
						return GL_DYNAMIC_COPY;
				}
		}
		// If the enums are ill-formed, default to static draw.
		return GL_STATIC_DRAW;
	}

	OGLGenericBufferImplicit::OGLGenericBufferImplicit() : buffer_handle(0)
	{
		glGenBuffers(1, &this->buffer_handle);
	}

	OGLGenericBufferImplicit::~OGLGenericBufferImplicit()
	{
		glDeleteBuffers(1, &this->buffer_handle);
	}

	bool OGLGenericBufferImplicit::is_mutable() const
	{
		switch (this->get_parameter<OGLBufferParameter::MUTABILITY>())
		{
			case GL_TRUE:
				return true;
			case GL_FALSE:
			default:
				return false;
		}
	}

	unsigned int OGLGenericBufferImplicit::get_size() const
	{
		return static_cast<unsigned int>(this->get_parameter<OGLBufferParameter::SIZE>());
	}

	bool OGLGenericBufferImplicit::empty() const
	{
		return this->get_size() == 0;
	}

	void OGLGenericBufferImplicit::allocate_memory(std::size_t size, const OGLBufferUsage& usage) const
	{
		glNamedBufferData(this->buffer_handle, size, nullptr, usage());
	}

	void OGLGenericBufferImplicit::update(GLintptr offset, GLsizeiptr size, const void* data) const
	{
		glNamedBufferSubData(this->buffer_handle, offset, size, data);
	}

	std::optional<OGLBufferUsage> OGLGenericBufferImplicit::query_current_usage() const
	{
		if(this->empty())
			return std::nullopt;
		return {OGLBufferUsage{static_cast<const GLenum>(this->get_parameter<OGLBufferParameter::USAGE>())}};
	}
}