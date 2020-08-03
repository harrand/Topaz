//
// Created by Harrand on 25/12/2019.
//

#include "gl/buffer.hpp"
#include "core/debug/assert.hpp"
#include <optional>

namespace tz::gl
{
	IBuffer::IBuffer(): handle(0)
	{
		glGenBuffers(1, &this->handle);
	}

	IBuffer::IBuffer(IBuffer&& move): handle(move.handle)
	{
		move.handle = 0;
	}

	IBuffer& IBuffer::operator=(IBuffer&& rhs)
	{
		std::swap(this->handle, rhs.handle);
		return *this;
	}

	IBuffer::~IBuffer()
	{
		glDeleteBuffers(1, &this->handle);
	}

	std::size_t IBuffer::size() const
	{
		IBuffer::verify();
		GLint param;
		glGetNamedBufferParameteriv(this->handle, GL_BUFFER_SIZE, &param);
		return static_cast<std::size_t>(param);
	}

	bool IBuffer::empty() const
	{
		return this->size() == 0;
	}

	bool IBuffer::is_terminal() const
	{
		IBuffer::verify();
		GLint param;
		glGetNamedBufferParameteriv(this->handle, GL_BUFFER_IMMUTABLE_STORAGE, &param);
		return (param == GL_FALSE) ? false : true;
	}

	bool IBuffer::valid() const
	{
		this->verify();
		return glIsBuffer(this->handle);
	}

	void IBuffer::resize(std::size_t size_bytes, BufferUsage usage)
	{
		IBuffer::verify();
		topaz_assert(!this->is_terminal(), "tz::gl::Buffer<T>::resize(", size_bytes, "): Cannot resize a terminal buffer.");
		topaz_assert(!this->is_mapped(), "tz::gl::Buffer<T>::resize(", size_bytes, "): Cannot resize because this buffer is currently mapped.");
		glNamedBufferData(this->handle, size_bytes, nullptr, static_cast<GLenum>(usage));
	}

	void IBuffer::safe_resize(std::size_t size_bytes)
	{
		if(this->size() == size_bytes)
			return;
		IBuffer::verify();
		topaz_assert(!this->is_mapped(), "tz::gl::Buffer<T>::safe_resize(", size_bytes, "): Cannot resize because this buffer is currently mapped.");
		topaz_assert(!this->is_terminal(), "tz::gl::Buffer<T>::safe_resize(", size_bytes, "): Cannot safe-resize a terminal buffer if the new size parameter is not equal to its old size.");
		std::size_t temp_size = this->size();
		void* temp_data = std::malloc(temp_size);
		this->retrieve_all(temp_data);
		this->resize(size_bytes);
		if(size_bytes > temp_size)
		{
			// if we now have extra space that is unused, just copy everything we have so far.
			tz::mem::Block blk{temp_data, temp_size};
			this->send(0, blk);
			// remainder after this block is now undefined and is free to be overwritten.
		}
		else
		{
			// just send as much as it can take
			this->send(temp_data);
		}
		std::free(temp_data);
	}

	void IBuffer::retrieve(std::size_t offset, std::size_t size_bytes, void* input_data) const
	{
		IBuffer::verify();
		if(!this->is_terminal())
		{
			// Cannot do this while mapped if we're non-terminal.
			topaz_assert(!this->is_mapped(), "tz::gl::IBuffer<T>::retrieve(", offset, ", ", size_bytes, ", ptr): Cannot retrieve because this buffer is both non-terminal and mapped. Cannot retrieve a non-terminal buffer if it is mapped.");
		}
		glGetNamedBufferSubData(this->handle, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size_bytes), input_data);
	}

	void IBuffer::retrieve_all(void* input_data) const
	{
		this->retrieve(0, this->size(), input_data);
	}

	void IBuffer::send(std::size_t offset, tz::mem::Block output_block)
	{
		IBuffer::verify();
		if(!this->is_terminal())
			topaz_assert(!this->is_mapped(), "tz::gl::Buffer::send(", offset, ", tz::mem::Block (", output_block.size(), ")): Cannot send because this buffer is both non-terminal and mapped. Cannot send data to a non-terminal buffer if it is mapped.");
		topaz_assert(output_block.size() <= (this->size() - offset), "tz::gl::Buffer::send(", offset, ", tz::mem::Block (", output_block.size(), ")): Block of size ", output_block.size(), " cannot fit in the buffer of size ", this->size(), " at the offset ", offset);
		glNamedBufferSubData(this->handle, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(output_block.size()), output_block.begin);
	}

	void IBuffer::send(const void* output_data)
	{
		IBuffer::verify();
		if(!this->is_terminal())
			topaz_assert(!this->is_mapped(), "tz::gl::Buffer::send(void*): Cannot send because this buffer is both non-terminal and mapped. Cannot send data to a non-terminal buffer if it is mapped.");
		// This is alot less safe for obvious reasons. But it's very low overhead so we should support it.
		glNamedBufferSubData(this->handle, 0, static_cast<GLsizeiptr>(this->size()), output_data);
	}

	void IBuffer::terminal_resize(std::size_t size_bytes)
	{
		IBuffer::verify();
		IBuffer::verify_nonterminal();
		topaz_assert(!this->is_mapped(), "tz::gl::Buffer<T>::terminal_resize(", size_bytes, "): Cannot resize because this buffer is currently mapped.");
		glNamedBufferStorage(this->handle, size_bytes, nullptr, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	}

	void IBuffer::make_terminal()
	{
		IBuffer::verify();
		IBuffer::verify_nonterminal();
		topaz_assert(!this->is_mapped(), "tz::gl::SSBO<T>::make_terminal(): Cannot make terminal because the buffer is currently mapped.");
		// Maintain a copy of the underlying data first and copy that data back into the immutable data store.
		auto sz = this->size();
		tz::mem::Block data_store{std::malloc(sz), sz};
		this->retrieve(0, sz, data_store.begin);
		glNamedBufferStorage(this->handle, sz, data_store.begin, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
		std::free(data_store.begin);
	}

	tz::mem::Block IBuffer::map(MappingPurpose purpose)
	{
		static std::optional<tz::mem::Block> block_cache;
		IBuffer::verify();
		if(this->is_mapped())
		{
			topaz_assert(block_cache.has_value(), "tz::gl::Buffer<T>::map(...): Attempted to map but we are already mapped without any cached result.");
			return block_cache.value();
		}
		// We know for sure that we have a valid handle, it is currently bound and we're definitely not yet mapped.
		void* begin = nullptr;
		if(this->is_terminal())
			begin = glMapNamedBufferRange(this->handle, 0, this->size(), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
		else
			begin = glMapNamedBuffer(this->handle, static_cast<GLenum>(purpose));
		tz::mem::Block blk{begin, this->size()};
		block_cache = blk;
		return blk;
	}

	void IBuffer::unmap()
	{
		IBuffer::verify();
		topaz_assert(this->is_mapped(), "tz::gl::Buffer<T>::unmap(): Attempted to unmap but we weren't already mapped");
		// We know for sure that we have a valid handle, it is currently bound and we're definitely mapped.
		glUnmapNamedBuffer(this->handle);
	}

	bool IBuffer::is_mapped() const
	{
		IBuffer::verify();
		GLint param;
		glGetNamedBufferParameteriv(this->handle, GL_BUFFER_MAPPED, &param);
		return param == GL_TRUE ? true : false;
	}
	
	bool IBuffer::operator==(BufferHandle handle) const
	{
		return this->handle == handle;
	}

	bool IBuffer::operator!=(BufferHandle handle) const
	{
		return this->handle != handle;
	}

	void IBuffer::verify() const
	{
		#if TOPAZ_DEBUG
			topaz_assert(this->handle != 0, "IBuffer::verify(): Handle was invalid (", this->handle, "). Perhaps this was moved from?");
		#endif
	}

	void IBuffer::verify_nonterminal() const
	{
		#if TOPAZ_DEBUG
			topaz_assert(!this->is_terminal(), "IBuffer::verify_nonterminal(): Buffer is terminal!");
		#endif
	}

	// Remember: SSBO == Buffer<BufferType::ShaderStorage>
	SSBO::Buffer(std::size_t layout_qualifier_id): IBuffer(), layout_qualifier_id(layout_qualifier_id)
	{
		this->bind();
		this->unbind();
	}

	SSBO::Buffer(SSBO&& move): IBuffer(std::move(move)){}
	
	SSBO& SSBO::operator=(SSBO&& rhs)
	{
		return static_cast<SSBO&>(IBuffer::operator=(std::move(rhs)));
	}

	void SSBO::bind() const
	{
		IBuffer::verify();
		constexpr GLenum type = static_cast<GLenum>(BufferType::ShaderStorage);
		glBindBuffer(type, this->handle);
		glBindBufferBase(type, this->layout_qualifier_id, this->handle);
	}

	void SSBO::unbind() const
	{
		IBuffer::verify();
		constexpr GLenum type = static_cast<GLenum>(BufferType::ShaderStorage);
		glBindBuffer(type, 0);
		glBindBufferBase(type, this->layout_qualifier_id, 0);
	}

	std::size_t SSBO::get_binding_id() const
	{
		return this->layout_qualifier_id;
	}

	// Remember: UBO == Buffer<BufferType::UniformStorage>
	UBO::Buffer(std::size_t layout_qualifier_id): IBuffer(), layout_qualifier_id(layout_qualifier_id)
	{
		this->bind();
		this->unbind();
	}

	void UBO::bind() const
	{
		IBuffer::verify();
		constexpr GLenum type = static_cast<GLenum>(BufferType::UniformStorage);
		glBindBuffer(type, this->handle);
		glBindBufferBase(type, this->layout_qualifier_id, this->handle);
	}

	void UBO::unbind() const
	{
		IBuffer::verify();
		constexpr GLenum type = static_cast<GLenum>(BufferType::UniformStorage);
		glBindBuffer(type, 0);
		glBindBufferBase(type, this->layout_qualifier_id, 0);
	}

	std::size_t UBO::get_binding_id() const
	{
		return this->layout_qualifier_id;
	}

	namespace bound
	{
		int vertex_buffer()
		{
			int handle;
			glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &handle);
			return handle;
		}

		int index_buffer()
		{
			int handle;
			glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &handle);
			return handle;
		}

		int shader_storage_buffer()
		{
			int handle;
			glGetIntegerv(GL_SHADER_STORAGE_BUFFER_BINDING, &handle);
			return handle;
		}

		int buffer(BufferType type)
		{
			switch(type)
			{
				case BufferType::Array:
					return vertex_buffer();
				break;
				case BufferType::Index:
					return index_buffer();
				break;
				case BufferType::ShaderStorage:
					return shader_storage_buffer();
				break;
				default:
					topaz_assert("tz::gl::bound::buffer(", static_cast<GLenum>(type), "): Checking the current bound buffer of this type is not yet implemented, or unsupported.");
					return 0;
				break;
			}
		}
	}
}