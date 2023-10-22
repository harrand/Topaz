#include "tz/ren/mesh2.hpp"
#include "tz/gl/draw.hpp"
#include "tz/gl/resource.hpp"
#include "tz/core/debug.hpp"
#include "tz/core/profile.hpp"

#include "tz/gl/imported_shaders.hpp"
#include ImportedShaderHeader(mesh, compute)

namespace tz::ren
{
//--------------------------------------------------------------------------------------------------
// vertex_wrangler
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
			rinfo.state().graphics.index_buffer = this->index_buffer;
		}

//--------------------------------------------------------------------------------------------------

		const tz::gl::iresource& vertex_wrangler::get_vertex_buffer(tz::gl::renderer_handle rh) const
		{
			tz::assert(rh != tz::nullhand);
			auto resptr = tz::gl::get_device().get_renderer(rh).get_resource(this->vertex_buffer);
			tz::assert(resptr != nullptr);
			return *resptr;
		}

//--------------------------------------------------------------------------------------------------

		tz::gl::iresource& vertex_wrangler::get_vertex_buffer(tz::gl::renderer_handle rh)
		{
			tz::assert(rh != tz::nullhand);
			auto resptr = tz::gl::get_device().get_renderer(rh).get_resource(this->vertex_buffer);
			tz::assert(resptr != nullptr);
			return *resptr;
		}

//--------------------------------------------------------------------------------------------------

		const tz::gl::iresource& vertex_wrangler::get_index_buffer(tz::gl::renderer_handle rh) const
		{
			tz::assert(rh != tz::nullhand);
			auto resptr = tz::gl::get_device().get_renderer(rh).get_resource(this->index_buffer);
			tz::assert(resptr != nullptr);
			return *resptr;
		}

//--------------------------------------------------------------------------------------------------

		tz::gl::iresource& vertex_wrangler::get_index_buffer(tz::gl::renderer_handle rh)
		{
			tz::assert(rh != tz::nullhand);
			auto resptr = tz::gl::get_device().get_renderer(rh).get_resource(this->index_buffer);
			tz::assert(resptr != nullptr);
			return *resptr;
		}

//--------------------------------------------------------------------------------------------------

		std::size_t vertex_wrangler::get_vertex_capacity(tz::gl::renderer_handle rh) const
		{
			return tz::gl::get_device().get_renderer(rh).get_resource(this->vertex_buffer)->data_as<const mesh_vertex>().size();
		}

//--------------------------------------------------------------------------------------------------

		std::size_t vertex_wrangler::get_index_capacity(tz::gl::renderer_handle rh) const
		{
			return tz::gl::get_device().get_renderer(rh).get_resource(this->index_buffer)->data_as<const mesh_index>().size();
		}

//--------------------------------------------------------------------------------------------------

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

//--------------------------------------------------------------------------------------------------

		void vertex_wrangler::remove_mesh(mesh_handle mh)
		{
			TZ_PROFZONE("vertex_wrangler - remove mesh", 0xFF02F3B5);
			auto hanval = static_cast<std::size_t>(static_cast<tz::hanval>(mh));
			// just add the handle to the free-list and empty out the corresponding locator.
			this->mesh_locators[hanval] = {};
			this->mesh_handle_free_list.push_back(static_cast<tz::hanval>(mh));
		}

//--------------------------------------------------------------------------------------------------

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

//--------------------------------------------------------------------------------------------------

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

//--------------------------------------------------------------------------------------------------

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

//--------------------------------------------------------------------------------------------------
// texture_manager
//--------------------------------------------------------------------------------------------------

		texture_manager::texture_manager(tz::gl::renderer_info& rinfo, std::size_t texture_capacity, tz::gl::resource_flags image_flags)
		{
			this->images.reserve(texture_capacity);
			for(std::size_t i = 0; i < texture_capacity; i++)
			{
				this->images.push_back(rinfo.add_resource
				(
					tz::gl::image_resource::from_uninitialised
					({
						.format = tz::gl::image_format::RGBA32,
						.dimensions = {1u, 1u},
						.flags = image_flags
					})
				));
			}
		}

//--------------------------------------------------------------------------------------------------

		texture_manager::texture_handle texture_manager::add_texture(tz::gl::renderer_handle rh, const tz::io::image& img)
		{
			return this->add_texture_impl(rh, tz::vec2ui{img.width, img.height}, img.data);
		}

//--------------------------------------------------------------------------------------------------

		void texture_manager::assign_texture(tz::gl::renderer_handle rh, texture_handle h, const tz::io::image& img)
		{
			this->assign_texture_impl(rh, h, tz::vec2ui{img.width, img.height}, img.data);
		}

//--------------------------------------------------------------------------------------------------

		texture_manager::texture_handle texture_manager::add_texture_impl(tz::gl::renderer_handle rh, tz::vec2ui dimensions, std::span<const std::byte> imgdata)
		{
			TZ_PROFZONE("texture_manager - add texture", 0xFFEEC2EA);
			#if TZ_DEBUG
				std::size_t sz = tz::gl::pixel_size_bytes(tz::gl::image_format::RGBA32) * dimensions[0] * dimensions[1];
				tz::assert(imgdata.size_bytes() == sz, "Unexpected image data length. Expected %zuB, but was %zuB", sz, imgdata.size_bytes());
			#endif
			tz::assert(this->texture_cursor < this->images.size(), "Ran out of textures. Limit: %zu", this->images.size());
			tz::gl::resource_handle imgh = this->images[this->texture_cursor];
			tz::gl::get_device().get_renderer(rh).edit
			({
				tz::gl::RendererEditBuilder{}
				.image_resize
				({
					.image_handle = imgh,
					.dimensions = dimensions,
				})
				.write
				({
					.resource = imgh,
					.data = imgdata
				})
				.build()
			});

			return static_cast<tz::hanval>(this->texture_cursor++);
		}

//--------------------------------------------------------------------------------------------------

		void texture_manager::assign_texture_impl(tz::gl::renderer_handle rh, texture_handle th, tz::vec2ui dimensions, std::span<const std::byte> imgdata)
		{
			TZ_PROFZONE("texture_manager - assign texture", 0xFFEEC2EA);
			auto hanval = static_cast<std::size_t>(static_cast<tz::hanval>(th));
			tz::gl::resource_handle imgh = this->images[hanval];

			tz::gl::get_device().get_renderer(rh).edit
			({
				tz::gl::RendererEditBuilder{}
				.image_resize
				({
					.image_handle = imgh,
					.dimensions = dimensions,
				})
				.write
				({
					.resource = imgh,
					.data = imgdata
				})
				.build()
			});
		}

//--------------------------------------------------------------------------------------------------
// compute_pass
//--------------------------------------------------------------------------------------------------

		compute_pass::compute_pass()
		{
			TZ_PROFZONE("compute_pass - create", 0xFF97B354);
			tz::gl::renderer_info cinfo;
			cinfo.shader().set_shader(tz::gl::shader_stage::compute, ImportedShaderSource(mesh, compute));

			struct draw_commands_initial_data
			{
				std::uint32_t count;
				std::array<tz::gl::draw_indexed_indirect_command, compute_pass::initial_max_draw_count> commands;
			};

			this->draw_indirect_buffer = cinfo.add_resource
			(
				tz::gl::buffer_resource::from_one
				(
					draw_commands_initial_data{},
					{
						.flags = {tz::gl::resource_flag::draw_indirect_buffer}
					}
				)	
			);

			struct mesh_locator_buffer_initial_data
			{
				std::uint32_t count;
				std::array<mesh_locator, compute_pass::initial_max_draw_count> meshes;
			};

			this->mesh_locator_buffer = cinfo.add_resource
			(
				tz::gl::buffer_resource::from_one
				(
					mesh_locator_buffer_initial_data{},
					{
						.access = tz::gl::resource_access::dynamic_access
					}
				)
			);
			cinfo.debug_name("Mesh Renderer - Compute Pass");

			this->compute = tz::gl::get_device().create_renderer(cinfo);
		}

//--------------------------------------------------------------------------------------------------

		tz::gl::renderer_handle compute_pass::get_compute_pass() const
		{
			return this->compute;
		}

//--------------------------------------------------------------------------------------------------

		std::size_t compute_pass::get_draw_count() const
		{
			TZ_PROFZONE("compute_pass - get draw count", 0xFF97B354);
			std::span<std::byte> meshloc_buffer_data = tz::gl::get_device().get_renderer(this->compute).get_resource(this->mesh_locator_buffer)->data();
			return *reinterpret_cast<std::uint32_t*>(meshloc_buffer_data.data());
		}

//--------------------------------------------------------------------------------------------------

		std::size_t compute_pass::get_draw_capacity() const
		{
			TZ_PROFZONE("compute_pass - get draw capacity", 0xFF97B354);
			std::span<std::byte> meshloc_buffer_data = tz::gl::get_device().get_renderer(this->compute).get_resource(this->mesh_locator_buffer)->data();
			// move sizeof(uint32_t) bytes forward in the meshloc buffer.
			// remember, meshloc buffer consists of the count and then an array of mesh locators.
			// the length of that array is our draw capacity.
			meshloc_buffer_data = meshloc_buffer_data.subspan(sizeof(std::uint32_t));
			tz::assert(meshloc_buffer_data.size_bytes() % sizeof(mesh_locator) == 0, "Mesh Locator buffer resource, post-count should have remaining size divisible by sizeof(mesh_locator) (%zu), but its size is %zu. Internal logic error.", sizeof(mesh_locator), meshloc_buffer_data.size_bytes());
			return meshloc_buffer_data.size_bytes() / sizeof(std::uint32_t);
		}

//--------------------------------------------------------------------------------------------------

		void compute_pass::set_draw_capacity(std::size_t new_capacity)
		{
			TZ_PROFZONE("compute_pass - set draw capacity", 0xFF97B354);
			const std::size_t old_capacity = this->get_draw_capacity();
			if(old_capacity == new_capacity)
			{
				return;
			}
			if(new_capacity < old_capacity)
			{
				// if we're shrinking, the draw-count better not be at capacity, or objects may accidentally be deleted.
				tz::assert(this->get_draw_count() < new_capacity, "Attempted to shrink draw capacity from %zu to %zu, but the draw count is %zu - meaning that some objects would be corrupted. Remove the excess objects before shrinking the draw list!", old_capacity, new_capacity, this->get_draw_count());
			}
			const std::size_t old_draw_indirect_buffer_size = sizeof(std::uint32_t) + (old_capacity * sizeof(tz::gl::draw_indexed_indirect_command));
			const std::size_t old_mesh_locator_buffer_size = sizeof(std::uint32_t) + (old_capacity * sizeof(mesh_locator));
			// debug sanity check first.
			#if TZ_DEBUG
				auto& ren = tz::gl::get_device().get_renderer(this->compute);
				const std::size_t old_draw_indirect_buffer_actual_size = ren.get_resource(this->draw_indirect_buffer)->data().size_bytes();
				const std::size_t old_mesh_locator_buffer_actual_size = ren.get_resource(this->mesh_locator_buffer)->data().size_bytes();
				tz::assert(old_draw_indirect_buffer_size == old_draw_indirect_buffer_actual_size, "Debug sanity check failed. Old capacity of draw indirect buffer was expected to be %zu, but it is %zu. Is there some unexpected padding? Serious logic error.", old_draw_indirect_buffer_size, old_draw_indirect_buffer_actual_size);
				tz::assert(old_mesh_locator_buffer_size == old_mesh_locator_buffer_actual_size, "Debug sanity check failed. Old capacity of mesh locator buffer was expected to be %zu, but it is %zu. Is there some unexpected padding? Serious logic error.", old_mesh_locator_buffer_size, old_mesh_locator_buffer_actual_size);
			#endif

			const std::size_t new_draw_indirect_buffer_size = sizeof(std::uint32_t) + (new_capacity * sizeof(tz::gl::draw_indexed_indirect_command));
			const std::size_t new_mesh_locator_buffer_size = sizeof(std::uint32_t) + (new_capacity * sizeof(mesh_locator));

			tz::gl::get_device().get_renderer(this->compute).edit
			(
				tz::gl::RendererEditBuilder{}
				.buffer_resize
				({
					.buffer_handle = this->draw_indirect_buffer,
					.size = new_draw_indirect_buffer_size
				})
				.buffer_resize
				({
					.buffer_handle = this->mesh_locator_buffer,
					.size = new_mesh_locator_buffer_size
				})
				.build()
			);
		}

//--------------------------------------------------------------------------------------------------

		void compute_pass::set_mesh_at(std::size_t draw_id, mesh_locator loc)
		{
			TZ_PROFZONE("compute_pass - set mesh at", 0xFF97B354);
			std::span<std::byte> meshloc_buffer_data = tz::gl::get_device().get_renderer(this->compute).get_resource(this->mesh_locator_buffer)->data();
			// move sizeof(uint32_t) bytes forward in the meshloc buffer.
			// remember, meshloc buffer consists of the count and then an array of mesh locators.
			// the length of that array is our draw capacity.
			meshloc_buffer_data = meshloc_buffer_data.subspan(sizeof(std::uint32_t));
			tz::assert(meshloc_buffer_data.size_bytes() % sizeof(mesh_locator) == 0, "Mesh Locator buffer resource, post-count should have remaining size divisible by sizeof(mesh_locator) (%zu), but its size is %zu. Internal logic error.", sizeof(mesh_locator), meshloc_buffer_data.size_bytes());
			auto* loc_array = reinterpret_cast<mesh_locator*>(meshloc_buffer_data.data());
			tz::assert(draw_id < this->get_draw_count(), "Attempted to set mesh at draw-id %zu, but the draw-count was only %zu. You can only set mesh at a previously-assigned draw-id (e.g via add_object)");
			tz::assert(draw_id < this->get_draw_capacity(), "Attempted to set mesh at draw-id %zu, but the draw-capacity is %zu. You're probably mis-using the API", draw_id, this->get_draw_capacity());
			loc_array[draw_id] = loc;
		}

//--------------------------------------------------------------------------------------------------

		std::size_t compute_pass::add_new_draws(std::size_t number_of_new_draws)
		{
			TZ_PROFZONE("compute_pass - add new draws", 0xFF97B354);
			const std::size_t cap = this->get_draw_capacity();
			const std::size_t count = this->get_draw_count();
			if(count + number_of_new_draws >= cap)
			{
				// we're gonna run out of draws
				// how many more do we want?
				const std::size_t option_a = cap * 2;
				const std::size_t option_b = cap + number_of_new_draws;
				const std::size_t new_capacity = std::max(option_a, option_b);
				// do the resize.
				this->set_draw_capacity(new_capacity);
			}
			// we're guaranteed to have enough space, increment the draw count.
			this->set_draw_count(count + number_of_new_draws);
			// write empty mesh locators in the new spots. i *think* they should all be empty locators, but best to guarantee!
			for(std::size_t i = 0; i < number_of_new_draws; i++)
			{
				this->set_mesh_at(count + i, mesh_locator{});
			}
			return count;
		}

//--------------------------------------------------------------------------------------------------

		void compute_pass::set_draw_count(std::size_t new_draw_count)
		{
			TZ_PROFZONE("compute_pass - set draw count", 0xFF97B354);
			const std::size_t old_count = this->get_draw_count();
			if(old_count == new_draw_count)
			{
				return;
			}
			// the buffers both start with a uint32, and then an array of other stuff.
			// as we only wanna change that uint32, we interpret both buffers as an array of that, and just write to the first.
			auto& ren = tz::gl::get_device().get_renderer(this->compute);
			ren.get_resource(this->draw_indirect_buffer)->data_as<std::uint32_t>().front() = new_draw_count;
			ren.get_resource(this->mesh_locator_buffer)->data_as<std::uint32_t>().front() = new_draw_count;
		}
	}	

}