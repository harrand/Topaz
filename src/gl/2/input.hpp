#include "gl/2/api/input.hpp"
#include "core/containers/basic_list.hpp"
#include "core/vector.hpp"

namespace tz::gl2
{
	/**
	 * @ingroup tz_gl2_io
	 * Standard-layout structure storing mesh data. Note that Meshes must always be indexed.
	 */
	struct Mesh
	{
		struct Vertex
		{
			tz::Vec3 position;
			tz::Vec2 texcoord;
			tz::Vec3 normal;
			tz::Vec3 tangent;
			tz::Vec3 bitangent;
		};

		using Index = unsigned int;

		/// Vertex data.
		tz::BasicList<Vertex> vertices;
		/// Index data.
		tz::BasicList<Index> indices;
	};

	/**
	 * @ingroup tz_gl2_io
	 * Represents a Mesh which is used as an input into a Renderer or Processor.
	 */
	class MeshInput : public IInput
	{
	public:
		MeshInput(Mesh data);
		virtual ~MeshInput() = default;
		// IInput
		virtual InputAccess get_access() const final;
		virtual std::span<const std::byte> vertex_data() const final;
		virtual std::span<std::byte> vertex_data() final;
		virtual std::span<const unsigned int> index_data() const final;
		virtual std::span<unsigned int> index_data() final;
		virtual std::unique_ptr<IInput> unique_clone() const final;
	private:
		Mesh data;
	};
}
