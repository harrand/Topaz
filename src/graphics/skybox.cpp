//
// Created by Harrand on 31/05/2018.
//

#include "skybox.hpp"

Skybox::Skybox(std::string cube_mesh_link, CubeMap& cm): cube_mesh_link(cube_mesh_link), cm(cm){}

void Skybox::render(const Camera& cam, Shader& shad, const Mesh& skybox_mesh, float width, float height)
{
    shad.bind();
    this->cm.bind(&shad, tz::graphics::texture_cubemap_sampler_id);
    shad.set_uniform<Matrix4x4>("m", tz::transform::model(cam.position, Vector3F(), Vector3F(cam.far_clip, cam.far_clip, cam.far_clip)));
    shad.set_uniform<Matrix4x4>("v", tz::transform::view(cam.position, cam.rotation));
    shad.set_uniform<Matrix4x4>("p", tz::transform::perspective_projection(cam.fov, width, height, cam.near_clip, cam.far_clip));
    shad.set_uniform<unsigned int>("shininess", 0);
    shad.set_uniform<float>("parallax_map_scale", 0);
    shad.set_uniform<float>("parallax_map_offset", 0);
    shad.set_uniform<float>("displacement_factor", 0);
    shad.update();
    glFrontFace(GL_CW);
    skybox_mesh.render(shad.has_tessellation_control_shader());
    glFrontFace(GL_CCW);
}