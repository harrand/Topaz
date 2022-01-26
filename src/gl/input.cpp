#include "gl/input.hpp"

namespace tz::gl2
{
	MeshInput::MeshInput(Mesh data):
	data(data){}

	InputAccess MeshInput::get_access() const
	{
		return InputAccess::StaticFixed;
	}

	std::span<const std::byte> MeshInput::vertex_data() const
	{
		return std::as_bytes(std::span<const Mesh::Vertex>(this->data.vertices));
	}

	std::span<std::byte> MeshInput::vertex_data()
	{
		return std::as_writable_bytes(std::span<Mesh::Vertex>(this->data.vertices));
	}

	std::span<const unsigned int> MeshInput::index_data() const
	{
		return this->data.indices;
	}

	std::span<unsigned int> MeshInput::index_data()
	{
		return this->data.indices;
	}

	std::unique_ptr<IInput> MeshInput::unique_clone() const
	{
		return std::make_unique<MeshInput>(*this);
	}
}
