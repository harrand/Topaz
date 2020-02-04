//
// Created by Harrand on 22/01/2020.
//

#include "test_framework.hpp"
#include "core/core.hpp"
#include "core/tz_glad/glad_context.hpp"
#include "gl/object.hpp"
#include "gl/manager.hpp"

tz::test::Case mock_mesh()
{
    tz::test::Case test_case("tz::gl::Manager Fake Mesh Tests");
    tz::gl::Manager mng;
    
    // Let's pretend we've loaded in a quad.
    tz::gl::MeshData quad;
    {
        // positions, texcoords, normals, tangents, bitangents
        // note: tangents and bitangents are undefined here.
        const tz::gl::Vertex vertices[] = {
            {{{-0.5f, -0.5f, 0.0f}}, {{0.0f, 0.0f}}, {{0.0f, 0.0f, -1.0f}}, {}, {}},
            {{{0.5f, -0.5f, -0.0f}}, {{1.0f, 0.0f}}, {{0.0f, 0.0f, -1.0f}}, {}, {}},
            {{{0.5f, 0.5f, 0.0f}}, {{1.0f, 1.0f}}, {{0.0f, 0.0f, -1.0f}}, {}, {}},
            {{{-0.5f, 0.5f, 0.0f}}, {{0.0f, 1.0f}}, {{0.0f, 0.0f, -1.0f}}, {}, {}},
        };

        for(std::size_t i = 0; i < 4; i++)
            quad.vertices.push_back(vertices[i]);
    }

    // Now we'll sort out our expected naming scheme.
    tz::gl::StandardDataRegionNames names
    {
        "quad_positions",
        "quad_texcoords",
        "quad_normals",
        "quad_tangents",
        "quad_bitangents"
    };
    mng.add_data(tz::gl::Data::Static, quad, names);
    tz::mem::OwningBlock positions = mng.get_data(tz::gl::Data::Static, "quad_positions");
    tz::mem::OwningBlock texcoords = mng.get_data(tz::gl::Data::Static, "quad_texcoords");
    tz::mem::OwningBlock normals = mng.get_data(tz::gl::Data::Static, "quad_normals");
    tz::mem::OwningBlock tangents = mng.get_data(tz::gl::Data::Static, "quad_tangents");
    tz::mem::OwningBlock bitangents = mng.get_data(tz::gl::Data::Static, "quad_bitangents");

    for(std::size_t i = 0; i < quad.vertices.size(); i++)
    {
        for(std::size_t j = 0; j < 3; j++)
        {
            float exp = quad.vertices[i].position[j];
            float act = *positions.get<float>((i * sizeof(float) * 3) + (j * sizeof(float)));
            topaz_expect(test_case, exp == act, "tz::gl::Manager::get_data returned garbage. Expected position value ", exp, ", but got ", act);
        }

        for(std::size_t j = 0; j < 2; j++)
        {
            float exp = quad.vertices[i].texture_coordinate[j];
            float act = *texcoords.get<float>((i * sizeof(float) * 2) + (j * sizeof(float)));
            topaz_expect(test_case, exp == act, "tz::gl::Manager::get_data returned garbage. Expected texcoord value ", exp, ", but got ", act);
        }

        for(std::size_t j = 0; j < 3; j++)
        {
            float exp = quad.vertices[i].normal[j];
            float act = *normals.get<float>((i * sizeof(float) * 3) + (j * sizeof(float)));
            topaz_expect(test_case, exp == act, "tz::gl::Manager::get_data returned garbage. Expected normal value ", exp, ", but got ", act);
        }

        for(std::size_t j = 0; j < 3; j++)
        {
            float exp = quad.vertices[i].tangent[j];
            float act = *tangents.get<float>((i * sizeof(float) * 3) + (j * sizeof(float)));
            topaz_expect(test_case, exp == act, "tz::gl::Manager::get_data returned garbage. Expected tangent value ", exp, ", but got ", act);
        }

        for(std::size_t j = 0; j < 3; j++)
        {
            float exp = quad.vertices[i].bi_tangent[j];
            float act = *bitangents.get<float>((i * sizeof(float) * 3) + (j * sizeof(float)));
            topaz_expect(test_case, exp == act, "tz::gl::Manager::get_data returned garbage. Expected bi-tangent value ", exp, ", but got ", act);
        }
    }
    return test_case;
}


int main()
{
    tz::test::Unit manager;

    // We require topaz to be initialised.
    {
        tz::core::initialise("Manager Tests");
        manager.add(mock_mesh());
        tz::core::terminate();
    }
    return manager.result();
}