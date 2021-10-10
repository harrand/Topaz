#include "gl/input.hpp"
#include <cstring>

namespace tz::gl
{
	MeshInput::MeshInput(Mesh mesh):
	MeshInput(mesh, MeshInputIgnoreField{})
	{

	}

	MeshInput::MeshInput(Mesh mesh, MeshInputIgnoreField ignores):
	mesh(mesh),
	ignores(ignores)
	{

	}

	RendererElementFormat MeshInput::get_format() const
	{
		tz::BasicList<RendererAttributeFormat> attributes;
		// Create the attribute formats if they're not ignored.

		if(!this->ignores.contains(MeshInputIgnoreFlag::PositionIgnore))
		{
			attributes.add(
			{
				.element_attribute_offset = offsetof(Vertex, position),
				.type = tz::gl::RendererComponentType::Float32x3
			});
		}

		if(!this->ignores.contains(MeshInputIgnoreFlag::TexcoordIgnore))
		{
			attributes.add(
			{
				.element_attribute_offset = offsetof(Vertex, texcoord),
				.type = tz::gl::RendererComponentType::Float32x2
			});
		}

		if(!this->ignores.contains(MeshInputIgnoreFlag::NormalIgnore))
		{
			attributes.add(
			{
				.element_attribute_offset = offsetof(Vertex, normal),
				.type = tz::gl::RendererComponentType::Float32x3
			});
		}

		if(!this->ignores.contains(MeshInputIgnoreFlag::TangentIgnore))
		{
			attributes.add(
			{
				.element_attribute_offset = offsetof(Vertex, tangent),
				.type = tz::gl::RendererComponentType::Float32x3
			});
		}

		if(!this->ignores.contains(MeshInputIgnoreFlag::BitangentIgnore))
		{
			attributes.add(
			{
				.element_attribute_offset = offsetof(Vertex, bitangent),
				.type = tz::gl::RendererComponentType::Float32x3
			});
		}
		return
		{
			.binding_size = sizeof(Vertex),
			.basis = tz::gl::RendererInputFrequency::PerVertexBasis,
			.binding_attributes = attributes
		};
	}

	std::span<const std::byte> MeshInput::get_vertex_bytes() const
	{
		return std::as_bytes(std::span<const Vertex>(this->mesh.vertices.begin(), this->mesh.vertices.end()));
	}

	std::span<const unsigned int> MeshInput::get_indices() const
	{
		return {this->mesh.indices.begin(), this->mesh.indices.end()};
	}

	MeshDynamicInput::MeshDynamicInput(Mesh mesh):
	MeshDynamicInput(mesh, MeshInputIgnoreField{}){}

	MeshDynamicInput::MeshDynamicInput(Mesh mesh, MeshInputIgnoreField ignores):
	initial_data(mesh),
	ignores(ignores),
	vertex_data(nullptr),
	index_data(nullptr)
	{}

	MeshDynamicInput::MeshDynamicInput(const MeshDynamicInput& copy):
	initial_data(copy.initial_data),
	ignores(copy.ignores),
	vertex_data(nullptr),
	index_data(nullptr)
	{
		tz_assert(copy.vertex_data == nullptr && copy.index_data == nullptr, "Cannot copy a MeshDynamicInput when resource data has been provided.");
	}

	MeshDynamicInput& MeshDynamicInput::operator=(const MeshDynamicInput& rhs)
	{
		this->initial_data = rhs.initial_data;
		this->ignores = rhs.ignores;
		this->vertex_data = nullptr;
		this->index_data = nullptr;
		tz_assert(rhs.vertex_data == nullptr && rhs.index_data == nullptr, "Cannot copy a MeshDynamicInput when resource data has been provided.");
		return *this;
	}

	RendererElementFormat MeshDynamicInput::get_format() const
	{
		tz::BasicList<RendererAttributeFormat> attributes;
		// Create the attribute formats if they're not ignored.

		if(!this->ignores.contains(MeshInputIgnoreFlag::PositionIgnore))
		{
			attributes.add(
			{
				.element_attribute_offset = offsetof(Vertex, position),
				.type = tz::gl::RendererComponentType::Float32x3
			});
		}

		if(!this->ignores.contains(MeshInputIgnoreFlag::TexcoordIgnore))
		{
			attributes.add(
			{
				.element_attribute_offset = offsetof(Vertex, texcoord),
				.type = tz::gl::RendererComponentType::Float32x2
			});
		}

		if(!this->ignores.contains(MeshInputIgnoreFlag::NormalIgnore))
		{
			attributes.add(
			{
				.element_attribute_offset = offsetof(Vertex, normal),
				.type = tz::gl::RendererComponentType::Float32x3
			});
		}

		if(!this->ignores.contains(MeshInputIgnoreFlag::TangentIgnore))
		{
			attributes.add(
			{
				.element_attribute_offset = offsetof(Vertex, tangent),
				.type = tz::gl::RendererComponentType::Float32x3
			});
		}

		if(!this->ignores.contains(MeshInputIgnoreFlag::BitangentIgnore))
		{
			attributes.add(
			{
				.element_attribute_offset = offsetof(Vertex, bitangent),
				.type = tz::gl::RendererComponentType::Float32x3
			});
		}
		return
		{
			.binding_size = sizeof(Vertex),
			.basis = tz::gl::RendererInputFrequency::PerVertexBasis,
			.binding_attributes = attributes
		};
	}

	std::span<const std::byte> MeshDynamicInput::get_vertex_bytes() const
	{
		if(this->vertex_data == nullptr)
		{
			return std::as_bytes(std::span<const Vertex>(this->initial_data.vertices.begin(), this->initial_data.vertices.end()));
		}
		return {this->vertex_data, this->vertex_data + (this->initial_data.vertices.length() * sizeof(tz::gl::Vertex))};
	}

	std::span<const unsigned int> MeshDynamicInput::get_indices() const
	{
		if(this->index_data == nullptr)
		{
			return this->initial_data.indices;
		}
		return {this->index_data, this->index_data + this->initial_data.indices.length()};
	}

	std::span<std::byte> MeshDynamicInput::get_vertex_bytes_dynamic()
	{
		// If no resource data has been provided, we can just return the initial data.
		if(this->vertex_data == nullptr)
		{
			return std::as_writable_bytes(std::span<Vertex>(this->initial_data.vertices.begin(), this->initial_data.vertices.end()));
		}
		return {this->vertex_data, this->vertex_data + (this->initial_data.vertices.length() * sizeof(Vertex))};
	}

	void MeshDynamicInput::set_vertex_data(std::byte* vertex_data)
	{
		auto vertices = this->get_vertex_bytes();
		this->vertex_data = vertex_data;
		std::memcpy(this->vertex_data, vertices.data(), vertices.size_bytes());
	}

	void MeshDynamicInput::set_index_data(unsigned int* index_data)
	{
		auto indices = this->get_indices();
		this->index_data = index_data;
		std::memcpy(this->index_data, indices.data(), indices.size_bytes());
	}
}