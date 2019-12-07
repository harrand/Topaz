#include "utility/functional.hpp"
#ifdef TOPAZ_OPENGL
namespace tz::gl
{
	template<typename T>
	void OGLVertexAttribute::define(GLint size, GLboolean normalised, GLsizei stride) const
	{
		using namespace ::tz::gl;
		if constexpr(std::is_same_v<T, short>)
		{
			this->direct_define(size, OGLVertexAttributeType::SHORT, normalised, stride);
		}
		else if constexpr(std::is_same_v<T, unsigned short>)
		{
			this->direct_define(size, OGLVertexAttributeType::USHORT, normalised, stride);
		}
		else if constexpr(std::is_same_v<T, int>)
		{
			this->direct_define(size, OGLVertexAttributeType::INT, normalised, stride);
		}
		else if constexpr(std::is_same_v<T, unsigned int>)
		{
			this->direct_define(size, OGLVertexAttributeType::UINT, normalised, stride);
		}
		else if constexpr(std::is_same_v<T, float>)
		{
			this->direct_define(size, OGLVertexAttributeType::FLOAT, normalised, stride);
		}
		else if constexpr(std::is_same_v<T, double>)
		{
			this->direct_define(size, OGLVertexAttributeType::DOUBLE, normalised, stride);
		}
		else
		{
			static_assert(std::is_void_v<T>, "OGLVertexAttribute::define<T>(std::size_t): T is not a valid type. Accepted types are short, ushort, int, uint, float and double.");
		}
	}

	template<typename OGLArrayType, typename... Args>
	OGLArrayType& OGLVertexArray::emplace(Args&&... args)
	{
		this->bind();
		using namespace tz::utility::functional;
		if constexpr(is_related<OGLVertexBuffer, OGLArrayType>())
		{
			this->vertex_buffers.push_back(std::make_unique<OGLArrayType>(std::forward<Args>(args)...));
			return *dynamic_cast<OGLArrayType*>(this->vertex_buffers.back().get());
		}
		else if constexpr(is_related<OGLIndexBuffer, OGLArrayType>())
		{
			if(this->index_buffer != nullptr)
				return *this->index_buffer;
			this->index_buffer = std::make_unique<OGLArrayType>(std::forward<Args>(args)...);
			return *this->index_buffer;
		}
		else if constexpr(is_related<OGLVertexAttribute, OGLArrayType>())
		{
			this->vertex_attributes.push_back(std::make_unique<OGLArrayType>(std::forward<Args>(args)...));
			return *dynamic_cast<OGLArrayType*>(this->vertex_attributes.back().get());
		}
		else
			static_assert(std::is_void<OGLArrayType>::value, "OGLVertexArray::emplace<T, Args...>: OGLArrayType has unsupported type. Must be either a subclass or equivalent to OGLVertexBuffer or OGLVertexAttribute.");
	}

	template<typename... Args>
	OGLVertexBuffer& OGLVertexArray::emplace_vertex_buffer(Args&&... args)
	{
		return this->emplace<OGLVertexBuffer>(std::forward<Args>(args)...);
	}

	template<typename... Args>
	OGLIndexBuffer& OGLVertexArray::emplace_index_buffer(Args&&... args)
	{
		return this->emplace<OGLIndexBuffer>(std::forward<Args>(args)...);
	}

	template<typename... Args>
	OGLVertexAttribute& OGLVertexArray::emplace_vertex_attribute(Args&&... args)
	{
		return this->emplace<OGLVertexAttribute>(std::forward<Args>(args)...);
	}

	template<template<typename> typename Container, typename Index>
	void OGLVertexArray::render(bool tessellation, GLsizei instance_count, Container<Index>* indices) const
	{
		this->bind();
		auto draw = [&](GLsizei num, const void* data)
		{
			if(tessellation)
			{
				glPatchParameteri(GL_PATCH_VERTICES, 3);
				if(instance_count > 0)
					glDrawElementsInstanced(GL_PATCHES, num, GL_UNSIGNED_INT, data, instance_count);
				else
					glDrawElements(GL_PATCHES, num, GL_UNSIGNED_INT, data);
			}
			else
			{
				if(instance_count > 0)
					glDrawElementsInstanced(GL_TRIANGLES, num, GL_UNSIGNED_INT, data, instance_count);
				glDrawElements(GL_TRIANGLES, num, GL_UNSIGNED_INT, data);
			}
		};

		// if indices == nullptr, then we assume the element array is filled.
		if(indices == nullptr)
		{
			const OGLIndexBuffer* element_array_buffer = this->get_element_array_buffer();
			/*
			std::cout << "element array buffer = " << element_array_buffer << "\n";
			std::cout << "num vertex buffers = " << this->vertex_buffers.size() << "\n";
			std::cout << "num attributes = " << this->vertex_attributes.size() << "\n";
			 */
			topaz_assert(element_array_buffer != nullptr, "OGLVertexArray::render<...>(...): No indices specified, but there is no index buffer attached to this VAO either; rendering is impossible.");
			auto number_of_elements = static_cast<GLsizei>(element_array_buffer->get_size());
			draw(number_of_elements, nullptr);
		}
		else
			draw(indices->size(), indices->data());
		this->unbind();
	}
}
#endif