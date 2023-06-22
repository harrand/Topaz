#include "tz/core/data/vector.hpp"
#include <vector>

struct vertex_t
{
	tz::vec3 pos;
	tz::vec2 texc;
	tz::vec3 nrm;
	tz::vec3 tang;
};

struct mesh_t
{
	std::vector<vertex_t> vertices = {};
	std::vector<std::uint32_t> indices = {};
};
