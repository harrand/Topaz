#include "assimp/scene.h"
#include <string>

namespace tz::ext::assimp
{
	class Scene
	{
	public:
		Scene(const std::string& filename);
		Scene(const Scene& copy) = delete;
		Scene(Scene&& move);

		std::size_t size_meshes() const;
		const aiMesh* get_mesh(std::size_t idx = 0) const;

		const aiMaterial* get_material(std::size_t material_idx) const;

		Scene& operator=(const Scene& rhs) = delete;
		Scene& operator=(Scene&& rhs);
		~Scene();
	private:
		const aiScene* scene;
	};
}