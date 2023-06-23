#include "tz/core/data/vector.hpp"
#include <vector>

struct vertex_t
{
	tz::vec3 pos;
	float pad0;
	tz::vec2 texc;
	float pad1[2];
	tz::vec3 nrm;
	float pad2;
	tz::vec3 tang;
	float pad3;
};

struct mesh_t
{
	std::vector<vertex_t> vertices = {};
	std::vector<std::uint32_t> indices = {};
};
