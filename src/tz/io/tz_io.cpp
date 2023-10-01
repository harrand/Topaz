#include "tz/io/tz_io.hpp"
#include "tz/core/imported_text.hpp"
#include "tz/lua/api.hpp"
#include ImportedTextHeader(tz_io, lua)

#include "tz/io/gltf.hpp"

namespace tz::io
{
	struct impl_tz_io_gltf
	{
		gltf data;
		int vertex_count(tz::lua::state& state)
		{
			unsigned int ret = 0;
			for(std::size_t i = 0; i < this->data.get_meshes().size(); i++)
			{
				for(std::size_t j = 0; j < this->data.get_meshes()[i].submeshes.size(); j++)
				{
					ret += this->data.get_submesh_vertex_data(i, j).vertices.size();
				}
			}
			state.stack_push_uint(ret);
			return 1;
		}

		int index_count(tz::lua::state& state)
		{
			unsigned int ret = 0;
			for(std::size_t i = 0; i < this->data.get_meshes().size(); i++)
			{
				for(std::size_t j = 0; j < this->data.get_meshes()[i].submeshes.size(); j++)
				{
					ret += this->data.get_submesh_vertex_data(i, j).indices.size();
				}
			}
			state.stack_push_uint(ret);
			return 1;
		}
	};

	LUA_CLASS_BEGIN(impl_tz_io_gltf)
		LUA_CLASS_METHODS_BEGIN
			LUA_METHOD(impl_tz_io_gltf, vertex_count)
			LUA_METHOD(impl_tz_io_gltf, index_count)
		LUA_CLASS_METHODS_END
	LUA_CLASS_END

	LUA_NAMESPACE_BEGIN(impl_tz_io)
		LUA_NAMESPACE_FUNC_BEGIN(gltf_load)
			auto [filename] = tz::lua::parse_args<std::string>(state);
			LUA_CLASS_PUSH(state, impl_tz_io_gltf, {.data = gltf::from_file(filename.c_str())});
			return 1;
		LUA_NAMESPACE_FUNC_END
	LUA_NAMESPACE_END

	void lua_initialise(tz::lua::state& state)
	{
		state.assign_emptytable("tz.io");
		state.new_type("impl_tz_io_gltf", LUA_CLASS_NAME(impl_tz_io_gltf)::registers);
		state.open_lib("impl_tz_io", LUA_NAMESPACE_NAME(impl_tz_io)::registers);
		state.execute("tz.io = impl_tz_io");

		std::string tz_io_lua_api{ImportedTextData(tz_io, lua)};
		state.execute(tz_io_lua_api.data());
	}
}