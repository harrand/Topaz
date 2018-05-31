//
// Created by Harrand on 31/05/2018.
//

#ifndef TOPAZ_SKYBOX_HPP
#define TOPAZ_SKYBOX_HPP

#include <camera.hpp>
#include "shader.hpp"
#include "mesh.hpp"
#include "texture.hpp"

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
    Skybox(std::string cube_mesh_link, CubeMap& cm);
    /**
     * Render this Skybox, given the container of Mesh assets containing the Skybox
     * @param cam - Camera through which to view the Skybox
     * @param shad - Shader with which to render the Skybox
     * @param all_meshes - Container of Mesh assets
     * @param width - Width of the viewport, in any unit proportional to pixels
     * @param height - Height of the viewport, in any unit proportional to pixels
     */
    void render(const Camera& cam, Shader& shad, const Mesh& skybox_mesh, float width, float height);
private:
    /// Path to the cube-map Mesh being used by this Skybox.
    std::string cube_mesh_link;
    /// Underlying CubeMap.
    CubeMap& cm;
};


#endif //TOPAZ_SKYBOX_HPP
