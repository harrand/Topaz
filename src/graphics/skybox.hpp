#ifndef TOPAZ_SKYBOX_HPP
#define TOPAZ_SKYBOX_HPP

#include "graphics/camera.hpp"
#include "graphics/shader.hpp"
#include "graphics/mesh.hpp"
#include "graphics/texture.hpp"

/**
* Wraps an OpenGL cubemap via a set of six textures.
* Use this to render skyboxes in a 3D world easily. Bring your own skybox shader though (Default one provided with Topaz is called 'skybox').
*/
class Skybox
{
public:
    /**
     * Construct a Skybox with all specifications.
     * @param cube_mesh_link - Path to the cube-map Mesh being used
     * @param cm - Existing CubeMap to utilise for this Skybox
     */
    Skybox(std::string cube_mesh_link, CubeMap& cm, Vector3F rotation = {});
    /**
     * Render this Skybox, given the container of Mesh assets containing the Skybox
     * @param cam - Camera through which to view the Skybox
     * @param shad - Shader with which to render the Skybox
     * @param all_meshes - Container of Mesh assets
     * @param width - Width of the viewport, in any unit proportional to pixels
     * @param height - Height of the viewport, in any unit proportional to pixels
     */
    void render(const Camera& cam, Shader& shad, float width, float height, const std::optional<Mesh>& skybox_mesh = std::nullopt);
private:
    /// Path to the cube-map Mesh being used by this Skybox.
    std::string cube_mesh_link;
    /// Underlying CubeMap.
    CubeMap& cm;
public:
    /// Rotation of the Skybox.
    Vector3F rotation;
private:
    /// Simple cube for the Skybox to use by default.
    Mesh cube;
};


#endif //TOPAZ_SKYBOX_HPP
