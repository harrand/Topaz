#ifndef TOPAZ_GL_INPUT_HPP
#define TOPAZ_GL_INPUT_HPP
#include "gl/mesh.hpp"

namespace tz::gl
{
	/**
	 * @brief If a shader does not explicitly use each mesh input attribute, a warning may be emitted by the runtime. Pass ignore flags for each attribute your shader won't reference to prevent this.
	 * @note This may become required behaviour later on, so it is highly recommended that you provide ignore flags as necessary.
	 * 
	 */
	enum class MeshInputIgnoreFlag
	{
		PositionIgnore,
		TexcoordIgnore,
		NormalIgnore,
		TangentIgnore,
		BitangentIgnore
	};

	using MeshInputIgnoreField = tz::EnumField<MeshInputIgnoreFlag>;

	/**
	 * @brief Renderer Input representing a typical mesh.
	 */
	class MeshInput : public IRendererInputCopyable<MeshInput>
	{
	public:
		MeshInput(Mesh mesh);
		MeshInput(Mesh mesh, MeshInputIgnoreField ignores);
		MeshInput(const MeshInput& copy) = default;

		virtual RendererElementFormat get_format() const final;
		virtual std::span<const std::byte> get_vertex_bytes() const final;
		virtual std::span<const unsigned int> get_indices() const final;
	private:
		Mesh mesh;
		MeshInputIgnoreField ignores;
	};

	class MeshDynamicInput : public IRendererDynamicInputCopyable<MeshDynamicInput>
	{
	public:
		MeshDynamicInput(Mesh mesh);
		MeshDynamicInput(Mesh mesh, MeshInputIgnoreField ignores);
		MeshDynamicInput(const MeshDynamicInput& copy);
		~MeshDynamicInput() = default;
		MeshDynamicInput& operator=(const MeshDynamicInput& rhs);

		virtual RendererElementFormat get_format() const final;
		virtual std::span<const std::byte> get_vertex_bytes() const final;
		virtual std::span<const unsigned int> get_indices() const final;

		// IRendererDynamicInput
		virtual std::span<std::byte> get_vertex_bytes_dynamic() final;
		virtual void set_vertex_data(std::byte* vertex_data) final;
		virtual void set_index_data(unsigned int* index_data) final;
	private:
		Mesh initial_data;
		MeshInputIgnoreField ignores;
		std::byte* vertex_data;
		unsigned int* index_data;
	};
}

#endif // TOPAZ_GL_INPUT_HPP