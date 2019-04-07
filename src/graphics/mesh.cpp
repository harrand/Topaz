#include "graphics/mesh.hpp"
#include "utility/functional.hpp"
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <map>
#include <unordered_set>

#ifdef TOPAZ_OPENGL
namespace tz::platform
{
    OGLMesh::OGLMesh(std::string filename, std::size_t scene_index) : OGLMesh()
    {
        const aiScene *scene = aiImportFile(filename.c_str(),aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_TransformUVCoords | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph);
        if (scene == nullptr)
        {
            tz::debug::print("OGLMesh::OGLMesh(filename, ...): Error: OGLMesh import failed:\n", aiGetErrorString(), "\n");
            return;
        }
        aiMesh *assimp_mesh = scene->mMeshes[scene_index];
        OGLMesh to_assign{assimp_mesh};
        aiReleaseImport(scene);
        *this = std::move(to_assign);
    }

    OGLMesh::OGLMesh(const Vertex *vertices, std::size_t number_of_vertices, const unsigned int *indices, std::size_t number_of_indices) : OGLMesh()
    {
        for (unsigned int i = 0; i < number_of_vertices; i++)
            this->vertices.push_back(vertices[i]);
        for (unsigned int i = 0; i < number_of_indices; i++)
            this->indices.push_back(indices[i]);
        this->init_mesh();
    }

    OGLMesh::OGLMesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices) : OGLMesh(vertices.data(), vertices.size(), indices.data(), indices.size()){}

    OGLMesh::OGLMesh(const OGLMesh &copy) : OGLMesh(copy.vertices, copy.indices) {}

    OGLMesh::OGLMesh(OGLMesh &&move) : vertices(std::move(move.vertices)), vertex_array(std::move(move.vertex_array)), indices(std::move(move.indices)) {}

    OGLMesh &OGLMesh::operator=(OGLMesh rhs)
    {
        OGLMesh::swap(*this, rhs);
        return *this;
    }

    std::vector<Vector3F> OGLMesh::get_positions() const
    {
        std::vector<Vector3F> positions;
        positions.reserve(this->vertices.size());
        for (const auto &vertex : this->vertices)
            positions.push_back(vertex.position);
        return positions;
    }

    std::vector<Vector2F> OGLMesh::get_texcoords() const
    {
        std::vector<Vector2F> texture_coordinates;
        texture_coordinates.reserve(this->vertices.size());
        for (const auto &vertex : this->vertices)
            texture_coordinates.push_back(vertex.texture_coordinate);
        return texture_coordinates;
    }

    std::vector<Vector3F> OGLMesh::get_normals() const
    {
        std::vector<Vector3F> normals;
        normals.reserve(this->vertices.size());
        for (const auto &vertex : this->vertices)
            normals.push_back(vertex.normal);
        return normals;
    }

    std::vector<Vector3F> OGLMesh::get_tangents() const
    {
        std::vector<Vector3F> tangents;
        tangents.reserve(this->vertices.size());
        for (const auto &vertex : this->vertices)
            tangents.push_back(vertex.tangent);
        return tangents;
    }

    const std::vector<unsigned int> &OGLMesh::get_indices() const
    {
        return this->indices;
    }

    void OGLMesh::render(bool patches) const
    {
        this->vertex_array.render(patches);
    }

    bool OGLMesh::operator==(const OGLMesh &rhs) const
    {
        return this->vertex_array == rhs.vertex_array;
    }

    OGLMesh::OGLMesh() : vertices(), vertex_array(), indices(){}

    OGLMesh::OGLMesh(const aiMesh *assimp_mesh) : OGLMesh()
    {
        /* things to assign:
         * position (vec3)
         * texcoord (vec2)
         * normal (vec3)
         * tangent (vec3)
         * render_count (uint)
         */
        // Right now, we don't know number of indices. We will find that out after we perform indexing.
        this->vertices.reserve(assimp_mesh->mNumVertices);
        for (std::size_t i = 0; i < assimp_mesh->mNumVertices; i++)
        {
            Vector3F position{assimp_mesh->mVertices[i].x, assimp_mesh->mVertices[i].y, assimp_mesh->mVertices[i].z};
            Vector2F texcoord = {};
            if (assimp_mesh->HasTextureCoords(0))
            {
                const aiVector3D *assimp_texcoord = &assimp_mesh->mTextureCoords[0][i];
                texcoord = {assimp_texcoord->x, assimp_texcoord->y};
            }
            Vector3F normal = {};
            if (assimp_mesh->HasNormals())
                normal = Vector3F{assimp_mesh->mNormals[i].x, assimp_mesh->mNormals[i].y, assimp_mesh->mNormals[i].z}.normalised();
            Vector3F tangent = {};
            if (assimp_mesh->HasTangentsAndBitangents())
                tangent = Vector3F{assimp_mesh->mTangents[i].x, assimp_mesh->mTangents[i].y, assimp_mesh->mTangents[i].z}.normalised();
            this->vertices.emplace_back(position, texcoord, normal, tangent);
        }
        for (std::size_t i = 0; i < assimp_mesh->mNumFaces; i++)
        {
            const aiFace &face = assimp_mesh->mFaces[i];
            this->indices.push_back(face.mIndices[0]);
            this->indices.push_back(face.mIndices[1]);
            this->indices.push_back(face.mIndices[2]);
        }
        this->init_mesh();
    }

    void OGLMesh::swap(OGLMesh &lhs, OGLMesh &rhs)
    {
        std::swap(lhs.vertices, rhs.vertices);
        tz::platform::OGLVertexArray::swap(lhs.vertex_array, rhs.vertex_array);
        std::swap(lhs.indices, rhs.indices);
    }

//private
    void OGLMesh::init_mesh()
    {
        using tz::graphics::BufferTypes;
        // Our vector class is not "c-enough" (contains stuff like protected variables which C structs don't support. Therefore we use VectorSXF instead which can work with OpenGL easily.
        // However our indices vector from model is fine as-is.
        std::vector<std::array<float, 3>> positions;
        positions.reserve(this->get_positions().size());
        std::vector<std::array<float, 2>> texcoords;
        texcoords.reserve(this->get_texcoords().size());
        std::vector<std::array<float, 3>> normals;
        normals.reserve(this->get_normals().size());
        std::vector<std::array<float, 3>> tangents;
        tangents.reserve(this->get_tangents().size());
        for (auto vec : this->get_positions())
            positions.push_back(vec.data());
        for (auto vec : this->get_texcoords())
            texcoords.push_back(vec.data());
        for (auto vec : this->get_normals())
            normals.push_back(vec.data());
        for (auto vec : this->get_tangents())
            tangents.push_back(vec.data());

        this->vertex_array.bind();

        // 0 = Vertices, 1 = Texture Coordinates, 2 = Internal Normals, 3 = Indices, 4 = Tangents
        using namespace tz::platform;
        using namespace tz::utility; // tz::utility::generic::sizeof_element
        OGLVertexBuffer &position_buffer = this->vertex_array.emplace_vertex_buffer(OGLVertexBufferTarget::ARRAY);
        auto default_usage = OGLVertexBufferUsage{OGLVertexBufferFrequency::STATIC, OGLVertexBufferNature::DRAW};
        position_buffer.insert(positions, default_usage);
        OGLVertexAttribute &position_attribute = this->vertex_array.emplace_vertex_attribute(0);
        position_attribute.define<float>(3, GL_FALSE, 3 * sizeof(float));
        position_buffer.unbind();

        OGLVertexBuffer &texcoord_buffer = this->vertex_array.emplace_vertex_buffer(OGLVertexBufferTarget::ARRAY);
        texcoord_buffer.insert(texcoords, default_usage);
        OGLVertexAttribute &texcoord_attribute = this->vertex_array.emplace_vertex_attribute(1);
        texcoord_attribute.define<float>(2, GL_FALSE, 2 * sizeof(float));
        texcoord_buffer.unbind();

        OGLVertexBuffer &normal_buffer = this->vertex_array.emplace_vertex_buffer(OGLVertexBufferTarget::ARRAY);
        normal_buffer.insert(normals, default_usage);
        OGLVertexAttribute &normal_attribute = this->vertex_array.emplace_vertex_attribute(2);
        normal_attribute.define<float>(3, GL_TRUE, 3 * sizeof(float));
        normal_buffer.unbind();

        OGLVertexBuffer &tangent_buffer = this->vertex_array.emplace_vertex_buffer(OGLVertexBufferTarget::ARRAY);
        tangent_buffer.insert(tangents, default_usage);
        OGLVertexAttribute &tangent_attribute = this->vertex_array.emplace_vertex_attribute(3);
        tangent_attribute.define<float>(3, GL_TRUE, 3 * sizeof(float));
        tangent_buffer.unbind();

        OGLVertexBuffer &index_buffer = this->vertex_array.emplace_vertex_buffer(OGLVertexBufferTarget::ELEMENT_ARRAY);
        index_buffer.insert(this->indices, default_usage);

        position_attribute.enable();
        texcoord_attribute.enable();
        normal_attribute.enable();
        tangent_attribute.enable();

        this->vertex_array.unbind();
    }

    OGLInstancedMesh::OGLInstancedMesh(std::string filename, std::vector<Vector3F> positions, std::vector<Vector3F> rotations, std::vector<Vector3F> scales, bool dynamic_transform): OGLInstancedMesh(OGLMesh{filename}, positions, rotations, scales, dynamic_transform){}

    OGLInstancedMesh::OGLInstancedMesh(const OGLMesh &uninstanced_copy, std::vector<Vector3F> positions, std::vector<Vector3F> rotations, std::vector<Vector3F> scales, bool dynamic_transform): OGLMesh(uninstanced_copy), positions(positions), rotations(rotations), scales(scales), models({}), instance_quantity(std::max({this->positions.size(), this->rotations.size(), this->scales.size()})), dynamic_transform(dynamic_transform), model_matrix_x_vbo(nullptr), model_matrix_y_vbo(nullptr), model_matrix_z_vbo(nullptr), model_matrix_w_vbo(nullptr)
    {
        // Build all of the model matrices.
        std::size_t number_of_matrices = positions.size();
        this->models.reserve(number_of_matrices);
        for (std::size_t i = 0; i < number_of_matrices; i++)
        {
            Vector3F position = this->positions[i];
            // Rotation and Scale aren't guaranteed to exist, so double check and default if not exist.
            Vector3F rotation, scale;
            try
            {
                rotation = this->rotations.at(i);
                scale = this->scales.at(i);
            }
            catch(const std::out_of_range &oor)
            {
                rotation = {};
                scale = {1, 1, 1};
            }
            this->models.push_back(tz::transform::model(position, rotation, scale));
        }

        // Foreach model matrix, populate each row so they can be individually sent to VRAM.
        std::vector<std::array<float, 4>> xs, ys, zs, ws;
        for (Matrix4x4 model : this->models)
        {
            xs.push_back(model.x.data());
            ys.push_back(model.y.data());
            zs.push_back(model.z.data());
            ws.push_back(model.w.data());
        }

        using namespace tz::platform;
        using namespace tz::utility; // tz::utility::generic::sizeof_element
        OGLVertexBufferUsage usage{OGLVertexBufferFrequency::STATIC, OGLVertexBufferNature::DRAW};
        if (dynamic_transform)
            usage = {OGLVertexBufferFrequency::DYNAMIC, OGLVertexBufferNature::DRAW};
        this->vertex_array.bind();
        // Row X (attribute 4)
        this->model_matrix_x_vbo = &this->vertex_array.emplace_vertex_buffer(OGLVertexBufferTarget::ARRAY);
        this->model_matrix_x_vbo->insert(xs, usage);
        OGLVertexAttribute &model_matrix_x_attribute = this->vertex_array.emplace_vertex_attribute(4);
        model_matrix_x_attribute.define<float>(4, GL_FALSE, 4 * sizeof(float));
        model_matrix_x_vbo->unbind();
        model_matrix_x_attribute.instanced_define(1);
        // Row Y (attribute 5)
        this->model_matrix_y_vbo = &this->vertex_array.emplace_vertex_buffer(OGLVertexBufferTarget::ARRAY);
        this->model_matrix_y_vbo->insert(ys, usage);
        OGLVertexAttribute &model_matrix_y_attribute = this->vertex_array.emplace_vertex_attribute(5);
        model_matrix_y_attribute.define<float>(4, GL_FALSE, 4 * sizeof(float));
        model_matrix_y_vbo->unbind();
        model_matrix_y_attribute.instanced_define(1);
        // Row Z (attribute 6)
        this->model_matrix_z_vbo = &this->vertex_array.emplace_vertex_buffer(OGLVertexBufferTarget::ARRAY);
        this->model_matrix_z_vbo->insert(zs, usage);
        OGLVertexAttribute &model_matrix_z_attribute = this->vertex_array.emplace_vertex_attribute(6);
        model_matrix_z_attribute.define<float>(4, GL_FALSE, 4 * sizeof(float));
        model_matrix_z_vbo->unbind();
        model_matrix_z_attribute.instanced_define(1);
        // Row W (attribute 7)
        this->model_matrix_w_vbo = &this->vertex_array.emplace_vertex_buffer(OGLVertexBufferTarget::ARRAY);
        this->model_matrix_w_vbo->insert(ws, usage);
        OGLVertexAttribute &model_matrix_w_attribute = this->vertex_array.emplace_vertex_attribute(7);
        model_matrix_w_attribute.define<float>(4, GL_FALSE, 4 * sizeof(float));
        model_matrix_w_vbo->unbind();
        model_matrix_w_attribute.instanced_define(1);

        model_matrix_x_attribute.enable();
        model_matrix_y_attribute.enable();
        model_matrix_z_attribute.enable();
        model_matrix_w_attribute.enable();
        // Finish working with our new vao.
        this->vertex_array.unbind();
    }

    OGLInstancedMesh::OGLInstancedMesh(const OGLInstancedMesh &copy) : OGLInstancedMesh(copy, copy.positions, copy.rotations, copy.scales, copy.dynamic_transform) {}

    OGLInstancedMesh::OGLInstancedMesh(OGLInstancedMesh &&move) : OGLMesh(move), positions(std::move(move.positions)), rotations(std::move(move.rotations)), scales(std::move(move.scales)), models(std::move(move.models)), instance_quantity(std::move(move.instance_quantity)), dynamic_transform(std::move(move.dynamic_transform)), model_matrix_x_vbo(std::move(move.model_matrix_x_vbo)), model_matrix_y_vbo(std::move(move.model_matrix_y_vbo)),model_matrix_z_vbo(std::move(move.model_matrix_z_vbo)), model_matrix_w_vbo(std::move(move.model_matrix_w_vbo)){}

    OGLInstancedMesh &OGLInstancedMesh::operator=(OGLInstancedMesh rhs)
    {
        OGLInstancedMesh::swap(*this, rhs);
        return *this;
    }

    const std::vector<Vector3F> &OGLInstancedMesh::get_instance_positions() const
    {
        return this->positions;
    }

    const std::vector<Vector3F> &OGLInstancedMesh::get_instance_rotations() const
    {
        return this->rotations;
    }

    const std::vector<Vector3F> &OGLInstancedMesh::get_instance_scales() const
    {
        return this->scales;
    }

    bool OGLInstancedMesh::set_instance_position(std::size_t instance_id, Vector3F position)
    {
        //std::cout << "editing position of instance id " << instance_id << " to be " << position << "\n";
        try
        {
            this->positions[instance_id] = position;
            this->update_instance(instance_id);
            return true;
        }
        catch(const std::out_of_range &out_of_range)
        {
            return false;
        }
    }

    bool OGLInstancedMesh::set_instance_rotation(std::size_t instance_id, Vector3F rotation)
    {
        try
        {
            this->rotations[instance_id] = rotation;
            this->update_instance(instance_id);
            return true;
        }
        catch(const std::out_of_range &out_of_range)
        {
            return false;
        }
    }

    bool OGLInstancedMesh::set_instance_scale(std::size_t instance_id, Vector3F scale)
    {
        try
        {
            this->scales[instance_id] = scale;
            this->update_instance(instance_id);
            return true;
        }
        catch (const std::out_of_range &out_of_range)
        {
            return false;
        }
    }

    const std::vector<Matrix4x4> &OGLInstancedMesh::get_model_matrices() const
    {
        return this->models;
    }

    std::size_t OGLInstancedMesh::get_instance_quantity() const
    {
        return this->instance_quantity;
    }

    void OGLInstancedMesh::render(bool patches) const
    {
        this->vertex_array.render(patches, this->instance_quantity);
    }

    void OGLInstancedMesh::update_instance(std::size_t instance_id)
    {
        Matrix4x4 old_model = this->models[instance_id];
        this->models[instance_id] = tz::transform::model(this->positions[instance_id], this->rotations[instance_id], this->scales[instance_id]);
        Matrix4x4 new_model = this->models[instance_id];
        bool x_change, y_change, z_change, w_change;
        x_change = old_model.x != new_model.x;
        y_change = old_model.y != new_model.y;
        z_change = old_model.z != new_model.z;
        w_change = old_model.w != new_model.w;
        bool any_change = x_change || y_change || z_change || w_change;
        if (any_change)
            this->vertex_array.bind();//glBindVertexArray(this->vertex_array_object);
        using namespace tz::utility;
        if (x_change)
        {
            std::array<float, 4> row_data = new_model.x.data();
            std::size_t array_size_bytes = row_data.size() * generic::sizeof_element(row_data);
            //glNamedBufferSubData(this->model_matrix_x_vbo, instance_id * array_size_bytes, array_size_bytes, row_data.data());
            this->model_matrix_x_vbo->update(instance_id * array_size_bytes, array_size_bytes, row_data.data());
        }
        if (y_change)
        {
            std::array<float, 4> row_data = new_model.y.data();
            std::size_t array_size_bytes = row_data.size() * generic::sizeof_element(row_data);
            //glNamedBufferSubData(this->model_matrix_y_vbo, instance_id * array_size_bytes, array_size_bytes, row_data.data());
            this->model_matrix_y_vbo->update(instance_id * array_size_bytes, array_size_bytes, row_data.data());
        }
        if (z_change)
        {
            std::array<float, 4> row_data = new_model.z.data();
            std::size_t array_size_bytes = row_data.size() * generic::sizeof_element(row_data);
            //glNamedBufferSubData(this->model_matrix_z_vbo, instance_id * array_size_bytes, array_size_bytes, row_data.data());
            this->model_matrix_z_vbo->update(instance_id * array_size_bytes, array_size_bytes, row_data.data());
        }
        if (w_change)
        {
            std::array<float, 4> row_data = new_model.w.data();
            std::size_t array_size_bytes = row_data.size() * generic::sizeof_element(row_data);
            //glNamedBufferSubData(this->model_matrix_w_vbo, instance_id * array_size_bytes, array_size_bytes, row_data.data());
            this->model_matrix_w_vbo->update(instance_id * array_size_bytes, array_size_bytes, row_data.data());
        }
        if (any_change)
            this->vertex_array.unbind();//glBindVertexArray(0);
    }

    void OGLInstancedMesh::swap(OGLInstancedMesh &lhs, OGLInstancedMesh &rhs)
    {
        OGLMesh::swap(lhs, rhs);
        std::swap(lhs.positions, rhs.positions);
        std::swap(lhs.rotations, rhs.rotations);
        std::swap(lhs.scales, rhs.scales);
        std::swap(lhs.models, rhs.models);
        std::swap(lhs.instance_quantity, rhs.instance_quantity);
        std::swap(lhs.model_matrix_x_vbo, rhs.model_matrix_x_vbo);
        std::swap(lhs.model_matrix_y_vbo, rhs.model_matrix_y_vbo);
        std::swap(lhs.model_matrix_z_vbo, rhs.model_matrix_z_vbo);
        std::swap(lhs.model_matrix_w_vbo, rhs.model_matrix_w_vbo);
    }
}
#endif

std::vector<Mesh> tz::graphics::load_all_meshes(const std::string& filename)
{
	std::vector<Mesh> meshes;
	const aiScene* scene = aiImportFile(filename.c_str(), aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_TransformUVCoords | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph);
	if(scene == nullptr)
	{
		tz::debug::print("tz::graphics::load_all_meshes(filename): Error: Mesh import failed:\n", aiGetErrorString(), "\n");
		return {};
	}
	unsigned int num_meshes = scene->mNumMeshes;
	aiReleaseImport(scene);
	for(unsigned int i = 0; i < num_meshes; i++)
		meshes.emplace_back(filename, i);
	return meshes;
}

bool tz::graphics::is_instanced(const Mesh* mesh)
{
	return tz::utility::functional::is_a<const Mesh, const InstancedMesh>(*mesh);
}

Mesh tz::graphics::create_quad(float x, float y, float width, float height)
{
	// Just create a very simple quad.
	std::array<Vertex, 4> vertices({Vertex({x - width, y - height, 0}, {}, {}), Vertex({x - width, y + height, 0}, {0, 1}, {}), Vertex({x + width, y + height, 0}, {1, 1}, {}), Vertex({x + width, y - height, 0}, {1, 0}, {})});
	std::array<unsigned int, 6> indices({0, 1, 2, 0, 2, 3});
	return {vertices.data(), vertices.size(), indices.data(), indices.size()};
}

Mesh tz::graphics::create_cube()
{
	std::vector<Vertex> vertices;
	vertices.emplace_back(Vector3F(-1, -1, -1), Vector2F(1, 0), Vector3F(0, 0, -1));
	vertices.emplace_back(Vector3F(-1, 1, -1), Vector2F(0, 0), Vector3F(0, 0, -1));
	vertices.emplace_back(Vector3F(1, 1, -1), Vector2F(0, 1), Vector3F(0, 0, -1));
	vertices.emplace_back(Vector3F(1, -1, -1), Vector2F(1, 1), Vector3F(0, 0, -1));

	vertices.emplace_back(Vector3F(-1, -1, 1), Vector2F(1, 0), Vector3F(0, 0, 1));
	vertices.emplace_back(Vector3F(-1, 1, 1), Vector2F(0, 0), Vector3F(0, 0, 1));
	vertices.emplace_back(Vector3F(1, 1, 1), Vector2F(0, 1), Vector3F(0, 0, 1));
	vertices.emplace_back(Vector3F(1, -1, 1), Vector2F(1, 1), Vector3F(0, 0, 1));

	vertices.emplace_back(Vector3F(-1, -1, -1), Vector2F(0, 1), Vector3F(0, -1, 0));
	vertices.emplace_back(Vector3F(-1, -1, 1), Vector2F(1, 1), Vector3F(0, -1, 0));
	vertices.emplace_back(Vector3F(1, -1, 1), Vector2F(1, 0), Vector3F(0, -1, 0));
	vertices.emplace_back(Vector3F(1, -1, -1), Vector2F(0, 0), Vector3F(0, -1, 0));

	vertices.emplace_back(Vector3F(-1, 1, -1), Vector2F(0, 1), Vector3F(0, 1, 0));
	vertices.emplace_back(Vector3F(-1, 1, 1), Vector2F(1, 1), Vector3F(0, 1, 0));
	vertices.emplace_back(Vector3F(1, 1, 1), Vector2F(1, 0), Vector3F(0, 1, 0));
	vertices.emplace_back(Vector3F(1, 1, -1), Vector2F(0, 0), Vector3F(0, 1, 0));

	vertices.emplace_back(Vector3F(-1, -1, -1), Vector2F(1, 1), Vector3F(-1, 0, 0));
	vertices.emplace_back(Vector3F(-1, -1, 1), Vector2F(1, 0), Vector3F(-1, 0, 0));
	vertices.emplace_back(Vector3F(-1, 1, 1), Vector2F(0, 0), Vector3F(-1, 0, 0));
	vertices.emplace_back(Vector3F(-1, 1, -1), Vector2F(0, 1), Vector3F(-1, 0, 0));

	vertices.emplace_back(Vector3F(1, -1, -1), Vector2F(1, 1), Vector3F(1, 0, 0));
	vertices.emplace_back(Vector3F(1, -1, 1), Vector2F(1, 0), Vector3F(1, 0, 0));
	vertices.emplace_back(Vector3F(1, 1, 1), Vector2F(0, 0), Vector3F(1, 0, 0));
	vertices.emplace_back(Vector3F(1, 1, -1), Vector2F(0, 1), Vector3F(1, 0, 0));
	return {vertices, {0, 1, 2, 0, 2, 3, 6, 5, 4, 7, 6, 4, 10, 9, 8, 11, 10, 8, 12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 22, 21, 20, 23, 22, 20}};
}