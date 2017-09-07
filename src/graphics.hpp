#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include "glew.h"
#include "utility.hpp"

namespace tz
{
	namespace graphics
	{
		extern bool has_context;
		extern bool initialised;
		inline void initialise()
		{
			if(!tz::graphics::has_context)
			{
				tz::util::log::warning("Initialisation of tz::graphics aborted: No OpenGL context has been setup yet.");
				return;
			}
			tz::util::log::message("OpenGL context detected, initialising tz::graphics...");
			GLenum status = glewInit();
			if(status != GLEW_OK)
				tz::util::log::error("Initialisation of GLEW failed.\n\tInitialisation of tz::graphics unsuccessful!");
			else
				tz::util::log::message("Initialisation of GLEW successful.\n\tInitialised tz::graphics via GLEW (OpenGL).");
			tz::graphics::initialised = true;
		}
		inline void terminate()
		{
			tz::util::log::message("Terminated tz::graphics via GLEW (OpenGL).");
		}
	}
}

struct OBJIndex
{
    unsigned int vertex_index;
    unsigned int uv_index;
    unsigned int normal_index;
    
    bool operator<(const OBJIndex& r) const { return vertex_index < r.vertex_index; }
};

class IndexedModel
{
public:
    std::vector<Vector3F> positions;
    std::vector<Vector2F> texcoords;
    std::vector<Vector3F> normals;
	std::vector<Vector3F> tangents;
    std::vector<unsigned int> indices;
    
    void calcNormals();
	void calcTangents();
};

class OBJModel
{
public:
    std::vector<OBJIndex> obj_indices;
    std::vector<Vector3F> vertices;
    std::vector<Vector2F> uvs;
    std::vector<Vector3F> normals;
    bool has_uvs;
    bool has_normals;
    
    OBJModel(const std::string& file_name);
    
    IndexedModel toIndexedModel();
private:
    unsigned int findLastVertexIndex(const std::vector<OBJIndex*>& index_lookup, const OBJIndex* current_index, const IndexedModel& result);
    void createOBJFace(const std::string& line);
    
    Vector2F parseOBJVector2F(const std::string& line);
    Vector3F parseOBJVector3F(const std::string& line);
    OBJIndex parseOBJIndex(const std::string& token, bool* has_uvs, bool* has_normals);
};

#endif