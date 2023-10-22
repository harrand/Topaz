#include "tz/ren/mesh2.hpp"
#include "tz/gl/resource.hpp"
#include "tz/core/debug.hpp"
#include "tz/core/profile.hpp"

namespace tz::ren
{
//--------------------------------------------------------------------------------------------------
	namespace impl
	{
		vertex_wrangler::vertex_wrangler(tz::gl::renderer_info& rinfo)
		{
			std::vector<mesh_vertex> initial_vertices = {};
			initial_vertices.resize(vertex_wrangler::initial_vertex_capacity);
			std::vector<mesh_index> initial_indices = {};
			initial_indices.resize(vertex_wrangler::initial_vertex_capacity);

			// create vertex and index buffer based on initial capacities.
			this->vertex_buffer = rinfo.add_resource
			(
				tz::gl::buffer_resource::from_many
				(
					initial_vertices
				)
			);

			this->index_buffer = rinfo.add_resource
			(
				tz::gl::buffer_resource::from_many
				(
					initial_indices,
					{
						.flags = {tz::gl::resource_flag::index_buffer}
					}
				)
			);
		}

		const tz::gl::iresource& vertex_wrangler::get_vertex_buffer(tz::gl::renderer_handle rh) const
		{
			tz::assert(rh != tz::nullhand);
			auto resptr = tz::gl::get_device().get_renderer(rh).get_resource(this->vertex_buffer);
			tz::assert(resptr != nullptr);
			return *resptr;
		}

		tz::gl::iresource& vertex_wrangler::get_vertex_buffer(tz::gl::renderer_handle rh)
		{
			tz::assert(rh != tz::nullhand);
			auto resptr = tz::gl::get_device().get_renderer(rh).get_resource(this->vertex_buffer);
			tz::assert(resptr != nullptr);
			return *resptr;
		}

		const tz::gl::iresource& vertex_wrangler::get_index_buffer(tz::gl::renderer_handle rh) const
		{
			tz::assert(rh != tz::nullhand);
			auto resptr = tz::gl::get_device().get_renderer(rh).get_resource(this->index_buffer);
			tz::assert(resptr != nullptr);
			return *resptr;
		}

		tz::gl::iresource& vertex_wrangler::get_index_buffer(tz::gl::renderer_handle rh)
		{
			tz::assert(rh != tz::nullhand);
			auto resptr = tz::gl::get_device().get_renderer(rh).get_resource(this->index_buffer);
			tz::assert(resptr != nullptr);
			return *resptr;
		}

		std::size_t vertex_wrangler::get_vertex_capacity(tz::gl::renderer_handle rh) const
		{
			return tz::gl::get_device().get_renderer(rh).get_resource(this->vertex_buffer)->data_as<const mesh_vertex>().size();
		}

		std::size_t vertex_wrangler::get_index_capacity(tz::gl::renderer_handle rh) const
		{
			return tz::gl::get_device().get_renderer(rh).get_resource(this->index_buffer)->data_as<const mesh_index>().size();
		}

		vertex_wrangler::mesh_handle vertex_wrangler::add_mesh(tz::gl::renderer_handle rh, mesh m)
		{
			TZ_PROFZONE("vertex_wrangler - add mesh", 0xFF02F3B5);
			std::size_t hanval = this->mesh_locators.size();
			// first get our mesh locator.
			mesh_locator loc = this->add_mesh_impl(rh, m);
			// either re-use an old mesh handle
			if(!this->mesh_handle_free_list.empty())
			{
				hanval = static_cast<std::size_t>(this->mesh_handle_free_list.front());
				this->mesh_handle_free_list.pop_front();
				tz::assert(this->mesh_locators[hanval] == mesh_locator{}, "mesh_locator id %zu (from free-list) was *not* an empty mesh. logic error", hanval);
				this->mesh_locators[hanval] = loc;
			}
			else // or create a new one.
			{
				this->mesh_locators.push_back(loc);
			}
			return static_cast<tz::hanval>(hanval);
		}

		void vertex_wrangler::remove_mesh(mesh_handle mh)
		{
			TZ_PROFZONE("vertex_wrangler - remove mesh", 0xFF02F3B5);
			auto hanval = static_cast<std::size_t>(static_cast<tz::hanval>(mh));
			// just add the handle to the free-list and empty out the corresponding locator.
			this->mesh_locators[hanval] = {};
			this->mesh_handle_free_list.push_back(static_cast<tz::hanval>(mh));
		}

		std::optional<std::uint32_t> vertex_wrangler::try_find_vertex_region(tz::gl::renderer_handle rh, std::size_t vertex_count) const
		{
			TZ_PROFZONE("vertex_wrangler - try find vertex region", 0xFF02F3B5);
			// Sort mesh locators by vertex offset
			std::vector<mesh_locator> sorted_meshes = this->mesh_locators;
			std::sort(sorted_meshes.begin(), sorted_meshes.end(),
					[](const mesh_locator& a, const mesh_locator& b) {
						return a.vertex_offset < b.vertex_offset;
					});
			// iterate through sorted mesh locators to find gaps.
			std::uint32_t current_offset = 0;
			for(const mesh_locator& loc : sorted_meshes)
			{
				std::uint32_t gap_size = loc.vertex_offset - current_offset;
				if(gap_size >= vertex_count)
				{
					return current_offset;
				}
				current_offset = loc.vertex_offset + loc.vertex_count;
			}

			// check if there's enough space at the end of the buffer.
			std::uint32_t last_mesh_end = sorted_meshes.empty() ? 0 : sorted_meshes.back().vertex_offset + sorted_meshes.back().vertex_count;
			if(this->get_vertex_capacity(rh) - last_mesh_end >= vertex_count)
			{
				return last_mesh_end;
			}
			// no gap large enough and no space at the end. return nullopt - you're gonna need to resize.
			return std::nullopt;
		}

		std::optional<std::uint32_t> vertex_wrangler::try_find_index_region(tz::gl::renderer_handle rh, std::size_t index_count) const
		{
			TZ_PROFZONE("vertex_wrangler - try find index region", 0xFF02F3B5);
			// Sort mesh locators by index offset
			std::vector<mesh_locator> sorted_meshes = this->mesh_locators;
			std::sort(sorted_meshes.begin(), sorted_meshes.end(),
					[](const mesh_locator& a, const mesh_locator& b) {
						return a.index_offset < b.index_offset;
					});
			// iterate through sorted mesh locators to find gaps.
			std::uint32_t current_offset = 0;
			for(const mesh_locator& loc : sorted_meshes)
			{
				std::uint32_t gap_size = loc.index_offset - current_offset;
				if(gap_size >= index_count)
				{
					return current_offset;
				}
				current_offset = loc.index_offset + loc.index_count;
			}

			// check if there's enough space at the end of the buffer.
			std::uint32_t last_mesh_end = sorted_meshes.empty() ? 0 : sorted_meshes.back().index_offset + sorted_meshes.back().index_count;
			if(this->get_vertex_capacity(rh) - last_mesh_end >= index_count)
			{
				return last_mesh_end;
			}
			// no gap large enough and no space at the end. return nullopt - you're gonna need to resize.
			return std::nullopt;
		}

		mesh_locator vertex_wrangler::add_mesh_impl(tz::gl::renderer_handle rh, const mesh& m)
		{
			TZ_PROFZONE("vertex_wrangler - add mesh impl", 0xFF02F3B5);
			tz::gl::RendererEditBuilder builder;
			bool edit_required = false;
			const std::size_t index_count = m.indices.size();
			const std::size_t vertex_count = m.vertices.size();

			// Section A: ensure that we have enough space.

			// if we don't have enough index space, add some more.
			// todo: double capacity instead of only adding what we need?
			auto maybe_index_section = this->try_find_index_region(rh, index_count);
			if(!maybe_index_section.has_value())
			{
				builder.buffer_resize
				({
					.buffer_handle = this->index_buffer,
					.size = (this->get_index_capacity(rh) + index_count) * sizeof(mesh_index)
				});
				edit_required = true;
			}

			// if we don't have enough vertex space, add some more.
			// todo: double capacity instead of only adding what we need?
			auto maybe_vertex_section = this->try_find_vertex_region(rh, vertex_count);
			if(!maybe_vertex_section.has_value())
			{
				builder.buffer_resize
				({
					.buffer_handle = this->vertex_buffer,
					.size = (this->get_vertex_capacity(rh) + vertex_count) * sizeof(mesh_vertex)
				});
				edit_required = true;
			}

			// if we needed to resize, do it now, and get the new sections.
			if(edit_required)
			{
				tz::gl::get_device().get_renderer(rh).edit(builder.build());
				// note: clear out builder as we will re-use it for resource writes.
				builder = {};
				maybe_index_section = this->try_find_index_region(rh, index_count);
				maybe_vertex_section = this->try_find_vertex_region(rh, vertex_count);
			}

			// if our logic was correct, any resizes that have or have not occurred shall ensure that we have enough index+vertex space, so we assert that is the case.
			tz::assert(maybe_index_section.has_value());
			tz::assert(maybe_vertex_section.has_value());

			// Section B: write the mesh data into the regions we gathered.
			std::span<const mesh_vertex> vertex_src = m.vertices;
			std::span<const mesh_index> index_src = m.indices;
			builder.write
			({
				.resource = this->index_buffer,
				.data = std::as_bytes(index_src),
				.offset = maybe_index_section.value() * sizeof(mesh_index)
			});
			builder.write
			({
				.resource = this->vertex_buffer,
				.data = std::as_bytes(vertex_src),
				.offset = maybe_vertex_section.value() * sizeof(mesh_vertex)
			});
			tz::gl::get_device().get_renderer(rh).edit(builder.build());

			std::uint32_t vertex_offset = this->added_vertex_count;
			this->added_vertex_count += vertex_count;

			return // mesh_locator
			{
				.vertex_offset = maybe_vertex_section.value(),
				.vertex_count = static_cast<std::uint32_t>(vertex_src.size()),
				.index_offset = maybe_index_section.value(),
				.index_count = static_cast<std::uint32_t>(index_src.size()),
				.max_index_value = vertex_offset
			};
		}
	}	
}