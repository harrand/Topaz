#include "tz/ren/mesh.hpp"
#include "tz/gl/draw.hpp"
#include "tz/gl/resource.hpp"
#include "tz/core/debug.hpp"
#include "tz/core/profile.hpp"

#include "tz/gl/imported_shaders.hpp"
#include ImportedShaderHeader(mesh, vertex)
#include ImportedShaderHeader(mesh, fragment)
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
			TZ_PROFZONE("vertex_wrangler - create", 0xFF02F3B5);
			// create buffer resources with initial size.
			// resizing is omega slow, but we dont want to eat tons of vram for tiny scenes.
			// hopefully the initial vertex capacity is a sane compromise!
			// (also buffer resources do not support zero size, so we gotta have something even with no vertex/index data!)
			std::vector<mesh_vertex> initial_vertices = {};
			initial_vertices.resize(vertex_wrangler::initial_vertex_capacity);
			std::vector<mesh_index> initial_indices = {};
			initial_indices.resize(vertex_wrangler::initial_vertex_capacity);

			// create vertex buffer.
			this->vertex_buffer = rinfo.add_resource
			(
				tz::gl::buffer_resource::from_many
				(
					initial_vertices
				)
			);

			// create index buffer.
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

		std::size_t vertex_wrangler::get_vertex_count() const
		{
			std::size_t vtx_count = 0;
			for(const mesh_locator& loc : this->mesh_locators)
			{
				vtx_count += loc.vertex_count;
			}
			return vtx_count;
		}

//--------------------------------------------------------------------------------------------------

		std::size_t vertex_wrangler::get_index_count() const
		{
			std::size_t idx_count = 0;
			for(const mesh_locator& loc : this->mesh_locators)
			{
				idx_count += loc.index_count;
			}
			return idx_count;
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

		const mesh_locator& vertex_wrangler::get_mesh(mesh_handle h) const
		{
			auto hanval = static_cast<std::size_t>(static_cast<tz::hanval>(h));
			tz::assert(hanval < this->mesh_locators.size(), "Invalid mesh handle %zu", hanval);
			return this->mesh_locators[hanval];
		}

//--------------------------------------------------------------------------------------------------

		vertex_wrangler::mesh_handle vertex_wrangler::try_find_mesh_handle(const mesh_locator& loc) const
		{
			auto iter = std::find(this->mesh_locators.begin(), this->mesh_locators.end(), loc);
			if(iter == this->mesh_locators.end() || loc == mesh_locator{})
			{
				return tz::nullhand;
			}
			return static_cast<tz::hanval>(std::distance(this->mesh_locators.begin(), iter));
		}

//--------------------------------------------------------------------------------------------------

		std::size_t vertex_wrangler::get_mesh_count(bool include_free_list) const
		{
			if(!include_free_list)
			{
				return this->mesh_locators.size() - this->mesh_handle_free_list.size();
			}
			return this->mesh_locators.size();
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
						return a.vertex_offset + a.vertex_count < b.vertex_offset + b.vertex_count;
					});
			// iterate through sorted mesh locators to find gaps.
			std::uint32_t current_offset = 0;
			for(const mesh_locator& loc : sorted_meshes)
			{
				tz::assert(loc.vertex_offset >= current_offset);
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
						return a.index_offset + a.index_count < b.index_offset + b.index_count;
					});
			// iterate through sorted mesh locators to find gaps.
			std::uint32_t current_offset = 0;
			for(const mesh_locator& loc : sorted_meshes)
			{
				tz::assert(loc.index_offset >= current_offset);
				std::uint32_t gap_size = loc.index_offset - current_offset;
				if(gap_size >= index_count)
				{
					return current_offset;
				}
				current_offset = loc.index_offset + loc.index_count;
			}

			// check if there's enough space at the end of the buffer.
			std::uint32_t last_mesh_end = sorted_meshes.empty() ? 0 : sorted_meshes.back().index_offset + sorted_meshes.back().index_count;
			const std::size_t idx_cap = this->get_index_capacity(rh);
			if(idx_cap - last_mesh_end >= index_count)
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
			auto maybe_index_section = this->try_find_index_region(rh, index_count);
			if(!maybe_index_section.has_value())
			{
				// either add the exact number of indices required or double the current capacity - whichever is greater.
				std::size_t new_size = (this->get_index_capacity(rh) + index_count) * sizeof(mesh_index);
				new_size = std::max(new_size, this->get_index_capacity(rh) * 2 * sizeof(mesh_index));
				builder.buffer_resize
				({
					.buffer_handle = this->index_buffer,
					.size = new_size
				});
				edit_required = true;
			}

			// if we don't have enough vertex space, add some more.
			auto maybe_vertex_section = this->try_find_vertex_region(rh, vertex_count);
			if(!maybe_vertex_section.has_value())
			{
				// either add the exact number of vertices required or double the current capacity - whichever is greater.
				std::size_t new_size = (this->get_vertex_capacity(rh) + vertex_count) * sizeof(mesh_vertex);
				new_size = std::max(new_size, this->get_vertex_capacity(rh) * 2 * sizeof(mesh_vertex));
				builder.buffer_resize
				({
					.buffer_handle = this->vertex_buffer,
					.size = new_size
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
			
			tz::report("Added mesh at index offset %zu and vertex offset %zu", index_src.size(), vertex_src.size());

			return // mesh_locator
			{
				.vertex_offset = maybe_vertex_section.value(),
				.vertex_count = static_cast<std::uint32_t>(vertex_src.size()),
				.index_offset = maybe_index_section.value(),
				.index_count = static_cast<std::uint32_t>(index_src.size())
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
						.flags = image_flags,
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

		std::size_t texture_manager::get_texture_count() const
		{
			return this->texture_cursor;
		}

//--------------------------------------------------------------------------------------------------

		std::size_t texture_manager::get_texture_capacity() const
		{
			return this->images.size();
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

			// create a visibility buffer. this is a bool per draw.
			// if the user wants to make a specific draw invisible, they can set this to false.
			std::array<std::uint32_t, compute_pass::initial_max_draw_count> initial_visibilities;
			std::fill(initial_visibilities.begin(), initial_visibilities.end(), true);
			this->draw_visibility_buffer = cinfo.add_resource
			(
				tz::gl::buffer_resource::from_one
				(
					initial_visibilities,
					{
						.access = tz::gl::resource_access::dynamic_access
					}
				)
			);
			cinfo.debug_name("Mesh Renderer 2.0 - Compute Pass");

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
			return meshloc_buffer_data.size_bytes() / sizeof(mesh_locator);
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
			#if TZ_DEBUG
				const std::size_t old_draw_indirect_buffer_size = sizeof(std::uint32_t) + (old_capacity * sizeof(tz::gl::draw_indexed_indirect_command));
				const std::size_t old_mesh_locator_buffer_size = sizeof(std::uint32_t) + (old_capacity * sizeof(mesh_locator));
				const std::size_t old_visibility_buffer_size = sizeof(std::uint32_t) * old_capacity;
				// debug sanity check first.
				auto& ren = tz::gl::get_device().get_renderer(this->compute);
				const std::size_t old_draw_indirect_buffer_actual_size = ren.get_resource(this->draw_indirect_buffer)->data().size_bytes();
				const std::size_t old_mesh_locator_buffer_actual_size = ren.get_resource(this->mesh_locator_buffer)->data().size_bytes();
				const std::size_t old_visibility_buffer_actual_size = ren.get_resource(this->draw_visibility_buffer)->data().size_bytes();
				tz::assert(old_draw_indirect_buffer_size == old_draw_indirect_buffer_actual_size, "Debug sanity check failed. Old capacity of draw indirect buffer was expected to be %zu, but it is %zu. Is there some unexpected padding? Serious logic error.", old_draw_indirect_buffer_size, old_draw_indirect_buffer_actual_size);
				tz::assert(old_mesh_locator_buffer_size == old_mesh_locator_buffer_actual_size, "Debug sanity check failed. Old capacity of mesh locator buffer was expected to be %zu, but it is %zu. Is there some unexpected padding? Serious logic error.", old_mesh_locator_buffer_size, old_mesh_locator_buffer_actual_size);
				tz::assert(old_visibility_buffer_size == old_visibility_buffer_actual_size, "Debug sanity check failed. Old capacity of draw visibility buffer was expected to be %zu, but it is %zu. Is there some unexpected padding? Serious logic error.", old_visibility_buffer_size, old_visibility_buffer_actual_size);
			#endif

			const std::size_t new_draw_indirect_buffer_size = sizeof(std::uint32_t) + (new_capacity * sizeof(tz::gl::draw_indexed_indirect_command));
			const std::size_t new_mesh_locator_buffer_size = sizeof(std::uint32_t) + (new_capacity * sizeof(mesh_locator));
			const std::size_t new_visibility_buffer_size = sizeof(std::uint32_t) * new_capacity;

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
				.buffer_resize
				({
					.buffer_handle = this->draw_visibility_buffer,
					.size = new_visibility_buffer_size
				})
				.build()
			);
		}

//--------------------------------------------------------------------------------------------------

		mesh_locator compute_pass::get_mesh_at(std::size_t draw_id) const
		{
			TZ_PROFZONE("compute_pass - get mesh at", 0xFF97B354);
			std::span<const std::byte> meshloc_buffer_data = tz::gl::get_device().get_renderer(this->compute).get_resource(this->mesh_locator_buffer)->data();
			// move sizeof(uint32_t) bytes forward in the meshloc buffer.
			// remember, meshloc buffer consists of the count and then an array of mesh locators.
			// the length of that array is our draw capacity.
			meshloc_buffer_data = meshloc_buffer_data.subspan(sizeof(std::uint32_t));
			tz::assert(meshloc_buffer_data.size_bytes() % sizeof(mesh_locator) == 0, "Mesh Locator buffer resource, post-count should have remaining size divisible by sizeof(mesh_locator) (%zu), but its size is %zu. Internal logic error.", sizeof(mesh_locator), meshloc_buffer_data.size_bytes());
			auto* loc_array = reinterpret_cast<const mesh_locator*>(meshloc_buffer_data.data());
			tz::assert(draw_id < this->get_draw_count(), "Attempted to set mesh at draw-id %zu, but the draw-count was only %zu. You can only set mesh at a previously-assigned draw-id (e.g via add_object)");
			tz::assert(draw_id < this->get_draw_capacity(), "Attempted to set mesh at draw-id %zu, but the draw-capacity is %zu. You're probably mis-using the API", draw_id, this->get_draw_capacity());
			return loc_array[draw_id];
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

		bool compute_pass::get_visibility_at(std::size_t draw_id) const
		{
			TZ_PROFZONE("compute_pass - get visibility at", 0xFF97B354);
			std::span<const std::uint32_t> visibility_buffer_data = tz::gl::get_device().get_renderer(this->compute).get_resource(this->draw_visibility_buffer)->data_as<const std::uint32_t>();
			tz::assert(draw_id < this->get_draw_count(), "Attempted to set visibility at draw-id %zu, but the draw-count was only %zu. You're probably mis-using the API.", draw_id, this->get_draw_count());
			return visibility_buffer_data[draw_id];
		}

//--------------------------------------------------------------------------------------------------

		void compute_pass::set_visibility_at(std::size_t draw_id, bool visible)
		{
			TZ_PROFZONE("compute_pass - set visibility at", 0xFF97B354);
			std::span<std::uint32_t> visibility_buffer_data = tz::gl::get_device().get_renderer(this->compute).get_resource(this->draw_visibility_buffer)->data_as<std::uint32_t>();
			tz::assert(draw_id < this->get_draw_count(), "Attempted to set visibility at draw-id %zu, but the draw-count was only %zu. You're probably mis-using the API.", draw_id, this->get_draw_count());
			visibility_buffer_data[draw_id] = visible;
		}

//--------------------------------------------------------------------------------------------------

		std::vector<std::size_t> compute_pass::add_new_draws(std::size_t number_of_new_draws)
		{
			TZ_PROFZONE("compute_pass - add new draws", 0xFF97B354);
			std::vector<std::size_t> ret = {};
			ret.reserve(number_of_new_draws);
			const std::size_t cap = this->get_draw_capacity();
			const std::size_t count = this->get_draw_count();

			// if free list is not empty, use as many slots as we can.
			const std::size_t free_list_reuse_count = std::min(number_of_new_draws, this->draw_id_free_list.size());
			for(std::size_t i = 0; i < free_list_reuse_count; i++)
			{
				const std::size_t reused_draw_id = this->draw_id_free_list[i];
				number_of_new_draws--;
				ret.push_back(reused_draw_id);
			}
			this->draw_id_free_list.erase(this->draw_id_free_list.begin(), this->draw_id_free_list.begin() + free_list_reuse_count);

			// number_of_new_draws has been reduced by the free-list re-use, and ret may already be partially filled.
			// its possible we still don't have enough capacity though:
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
				ret.push_back(count + i);
			}

			// finally, for each new draw, do some basic empty initialisation.
			for(std::size_t draw_id : ret)
			{
				this->set_mesh_at(draw_id, mesh_locator{});
				this->set_visibility_at(draw_id, true);
			}
			return ret;
		}

//--------------------------------------------------------------------------------------------------

		void compute_pass::remove_draw(std::size_t draw_id)
		{
			TZ_PROFZONE("compute_pass - remove draw", 0xFF97B354);
			this->set_mesh_at(draw_id, mesh_locator{});
			// disabling visibility is pretty unnecessary, as empty mesh locators are skipped over anyway.
			//this->set_visibility_at(draw_id, false);
			if(std::find(this->draw_id_free_list.begin(), this->draw_id_free_list.end(), draw_id) != this->draw_id_free_list.end())
			{
				return;
			}
			this->draw_id_free_list.push_back(draw_id);
		}

//--------------------------------------------------------------------------------------------------

		tz::gl::resource_handle compute_pass::get_draw_indirect_buffer() const
		{
			return this->draw_indirect_buffer;
		}

//--------------------------------------------------------------------------------------------------

		std::size_t compute_pass::get_draw_free_list_count() const
		{
			return this->draw_id_free_list.size();
		}

//--------------------------------------------------------------------------------------------------

		bool compute_pass::is_in_free_list(std::size_t draw_id) const
		{
			return std::find(this->draw_id_free_list.begin(), this->draw_id_free_list.end(), draw_id) != this->draw_id_free_list.end();
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

//--------------------------------------------------------------------------------------------------
// object_storage
//--------------------------------------------------------------------------------------------------

		object_storage::object_storage(tz::gl::renderer_info& rinfo)
		{
			TZ_PROFZONE("object_storage - create", 0xFFAB567B);
			std::array<object_data, compute_pass::initial_max_draw_count> initial_object_data = {};
			this->object_buffer = rinfo.add_resource
			(
				tz::gl::buffer_resource::from_one
				(
					initial_object_data,
					{
						.access = tz::gl::resource_access::dynamic_access
					}
				)
			);
		}

//--------------------------------------------------------------------------------------------------

		std::size_t object_storage::get_object_capacity(tz::gl::renderer_handle rh) const
		{
			return tz::gl::get_device().get_renderer(rh).get_resource(this->object_buffer)->data_as<const object_data>().size();
		}

//--------------------------------------------------------------------------------------------------

		void object_storage::set_object_capacity(tz::gl::renderer_handle rh, std::size_t new_capacity)
		{
			TZ_PROFZONE("object_storage - set object capacity", 0xFFAB567B);
			const std::size_t old_capacity = this->get_object_capacity(rh);
			// early out if we dont need to do anything.
			if(old_capacity == new_capacity)
			{
				return;
			}

			tz::gl::RendererEditBuilder builder;
			builder.buffer_resize
			({
				.buffer_handle = this->object_buffer,
				.size = new_capacity * sizeof(object_data)
			});
			// Note: If we're setting object capacity, we assume that mesh locator buffer (draw list) has already been resized.
			// We use that as a resource reference, so we need to mark all buffers as dirty here, or it will not rerecord
			// rendering commands and try to use the dead old buffer.
			builder.mark_dirty({.buffers = true});
			tz::gl::get_device().get_renderer(rh).edit(builder.build());
			tz::assert(this->get_object_capacity(rh) == new_capacity);
			if(new_capacity > old_capacity)
			{
				// explicitly assign any new objects to a default empty object_data.
				for(std::size_t i = old_capacity; i < new_capacity; i++)
				{
					this->get_object_internals(rh)[i] = object_data{};	
				}
			}
		}

//--------------------------------------------------------------------------------------------------

		std::span<const object_data> object_storage::get_object_internals(tz::gl::renderer_handle rh) const
		{
			return tz::gl::get_device().get_renderer(rh).get_resource(this->object_buffer)->data_as<const object_data>();
		}

//--------------------------------------------------------------------------------------------------

		std::span<object_data> object_storage::get_object_internals(tz::gl::renderer_handle rh)
		{
			return tz::gl::get_device().get_renderer(rh).get_resource(this->object_buffer)->data_as<object_data>();
		}

//--------------------------------------------------------------------------------------------------
// render_pass
//--------------------------------------------------------------------------------------------------

		render_pass::render_pass(render_pass::info i)
		{
			TZ_PROFZONE("render_pass - create", 0xFFF1F474);
			tz::gl::renderer_info rinfo;
			if(i.output != nullptr)
			{
				rinfo.set_output(*i.output);
			}
			if(i.custom_vertex_spirv.empty())
			{
				i.custom_vertex_spirv = ImportedShaderSource(mesh, vertex);
			}
			if(i.custom_fragment_spirv.empty())
			{
				i.custom_fragment_spirv = ImportedShaderSource(mesh, fragment);
			}
			rinfo.shader().set_shader(tz::gl::shader_stage::vertex, i.custom_vertex_spirv);
			rinfo.shader().set_shader(tz::gl::shader_stage::fragment, i.custom_fragment_spirv);
			rinfo.set_options(i.custom_options | tz::gl::renderer_option::draw_indirect_count);

			// order is:
			// vertex buffer
			// index buffer
			// object buffer
			// camera buffer
			// draw indirect buffer ref
			// <extra buffers>
			// textures

			this->vtx = vertex_wrangler(rinfo);
			this->obj = object_storage(rinfo);

			// TODO: replace with proper camera buffer.
			std::array<tz::mat4, 2> camera_initial_data;
			camera_initial_data[0] = tz::view({0.0f, 0.0f, 25.0f}, tz::vec3::zero());
			camera_initial_data[1] = tz::perspective(1.5708f, static_cast<float>(tz::window().get_dimensions()[0]) / tz::window().get_dimensions()[1], 0.1f, 1000.0f);
			this->camera_buffer = rinfo.add_resource(tz::gl::buffer_resource::from_one(camera_initial_data,
			{
				.access = tz::gl::resource_access::dynamic_access
			}));
			// vertex wrangler already set our index buffer, but we need to set the draw indirect buffer ourselves.
			// first retrieve it from compute pass
			this->draw_indirect_ref = rinfo.ref_resource(this->compute.get_compute_pass(), this->compute.get_draw_indirect_buffer());
			// then set it up.
			rinfo.state().graphics.draw_buffer = this->draw_indirect_ref;
			rinfo.state().graphics.culling = tz::gl::graphics_culling::back;

			// after that, add all the extra buffers the user requested.
			// we're not gonna do anything with these, just expose them to the user to deal with.
			if(i.extra_buffers.size())
			{
				for(std::size_t id = 0; id < i.extra_buffers.size(); id++)
				{
					tz::gl::resource_handle resh = rinfo.add_resource(i.extra_buffers[id]);
					auto hanval = static_cast<std::size_t>(static_cast<tz::hanval>(resh));
					if(id == 0)
					{
						this->extra_buf_hanval_first = hanval;
					}
					if(id == (i.extra_buffers.size() - 1))
					{
						this->extra_buf_hanval_last = hanval;
					}
				}
			}

			// finally, add textures.
			this->tex = texture_manager(rinfo, i.texture_capacity);
			rinfo.debug_name("Mesh Renderer 2.0 - Render Pass");

			this->render = tz::gl::get_device().create_renderer(rinfo);
		}

//--------------------------------------------------------------------------------------------------

		void render_pass::append_to_render_graph()
		{
			auto hanval_ch = static_cast<tz::gl::eid_t>(static_cast<tz::hanval>(this->compute.get_compute_pass()));
			auto hanval_rh = static_cast<tz::gl::eid_t>(static_cast<tz::hanval>(this->render));

			// compute pass happens first, then the render pass.
			tz::gl::get_device().render_graph().timeline.push_back(hanval_ch);
			tz::gl::get_device().render_graph().timeline.push_back(hanval_rh);
			// render pass depends on compute pass.
			tz::gl::get_device().render_graph().add_dependencies(this->render, this->compute.get_compute_pass());
		}

//--------------------------------------------------------------------------------------------------

		void render_pass::update()
		{
			TZ_PROFZONE("render_pass - update", 0xFFD1D454);
			this->tree.iterate_nodes([this](unsigned int node_id)
			{
				TZ_PROFZONE("mesh_renderer - note iterate", 0xFF0000AA);
				const auto& node = this->tree.get_node(node_id);
				this->obj.get_object_internals(this->render)[node.data].global_transform = this->tree.get_global_transform(node_id).matrix();
			}, true);
		}

//--------------------------------------------------------------------------------------------------

		std::size_t render_pass::get_mesh_count(bool include_free_list) const
		{
			return this->vtx.get_mesh_count(include_free_list);
		}

//--------------------------------------------------------------------------------------------------

		render_pass::mesh_handle render_pass::add_mesh(mesh m)
		{
			TZ_PROFZONE("render_pass - add mesh", 0xFFF1F474);
			return this->vtx.add_mesh(this->render, m);
		}

//--------------------------------------------------------------------------------------------------

		const mesh_locator& render_pass::get_mesh(mesh_handle m) const
		{
			return this->vtx.get_mesh(m);
		}

//--------------------------------------------------------------------------------------------------

		void render_pass::remove_mesh(mesh_handle m)
		{
			TZ_PROFZONE("render_pass - remove mesh", 0xFFF1F474);
			// note: vertex_wrangler removing the mesh simply empties its mesh locator and makes the vertex/index regions available again.
			// however, the draw list contains its own set of mesh_locators which are meant to be kept in-sync with the vertex_wrangler's list of locators.
			// therefore we must null out both.
			// first we go through all objects. if they use this mesh handle, set them to use an empty mesh locator (as the mesh at this handle is about to become an empty locator)
			for(std::size_t i = 0; i < this->compute.get_draw_count(); i++)
			{
				const auto& loc = this->compute.get_mesh_at(i);	
				if(this->vtx.try_find_mesh_handle(loc) == m)
				{
					this->compute.set_mesh_at(i, {});
				}
			}
			// secondly, tell the vertex_wrangler to remove the mesh. that sets its copy to the empty locator, *and* makes its vertex/index data available for a new mesh.
			this->vtx.remove_mesh(m);
		}

//--------------------------------------------------------------------------------------------------

		render_pass::texture_handle render_pass::add_texture(const tz::io::image& img)
		{
			TZ_PROFZONE("render_pass - add texture", 0xFFF1F474);
			return this->tex.add_texture(this->render, img);
		}

//--------------------------------------------------------------------------------------------------

		std::size_t render_pass::get_object_count(bool include_free_list) const
		{
			if(!include_free_list)
			{
				return this->compute.get_draw_count() - this->compute.get_draw_free_list_count();
			}
			return this->compute.get_draw_count();
		}

//--------------------------------------------------------------------------------------------------

		render_pass::object_handle render_pass::add_object(object_create_info create)
		{
			TZ_PROFZONE("render_pass - add object", 0xFFF1F474);
			// add a new draw to the indirect buffer and the mesh locator buffer.
			std::size_t old_count = this->compute.get_draw_count();
			std::size_t old_capacity = this->compute.get_draw_capacity();
			// remember: add new draws will increase the compute's draw count automatically.
			// it will also increase the capacity if it needs to.
			// note for free-list. will definitely need to add logic for that in compute pass.
			// shouldn't need to do anything else here? just use the id we gave you and assume its valid.
			std::size_t our_object_id = this->compute.add_new_draws(1).front();
			if(old_count + 1 >= old_capacity && old_capacity != this->compute.get_draw_capacity())
			{
				// we're over capacity.
				// add_new_draws would've increased the capacity in some way.
				// we will need to set the object buffer's capacity to match this.
				std::size_t new_capacity = this->compute.get_draw_capacity();
				this->obj.set_object_capacity(this->render, new_capacity);
			}

			// we can now assume our object/mesh locator is ready to go, but is just empty and defaulted.
			object_data& data = this->obj.get_object_internals(this->render)[our_object_id];
			mesh_locator our_mesh = {};
			if(create.mesh != tz::nullhand)
			{
				our_mesh = this->vtx.get_mesh(create.mesh);
			}

			data.colour_tint = create.colour_tint;
			tz::assert(create.bound_textures.size() <= object_data::max_bound_textures, "add_object attempted with %zu bound textures. i'm afraid the implementation only supports %zu", create.bound_textures.size(), object_data::max_bound_textures);
			const std::size_t bound_tex_count = std::min(create.bound_textures.size(), object_data::max_bound_textures);
			for(std::size_t i = 0; i < bound_tex_count; i++)
			{
				data.bound_textures[i] = create.bound_textures[i];
			}
			// todo: calculate global transform asap? if we wait till next frame, the user could feasibly notice this new object at the origin for the remainder of this frame.
			// tell the compute pass about the mesh we want.
			this->compute.set_mesh_at(our_object_id, our_mesh);
			this->compute.set_visibility_at(our_object_id, create.is_visible);

			// finally, add this object as a node into our transform hierarchy.
			if(create.parent != tz::nullhand)
			{
				std::optional<unsigned int> parent_node_id = this->tree.find_node(static_cast<std::size_t>(static_cast<tz::hanval>(create.parent)));
				tz::assert(parent_node_id.has_value(), "add_object provided parent that was invalid.");
				this->tree.add_node(create.local_transform, our_object_id, parent_node_id);
			}
			else
			{
				this->tree.add_node(create.local_transform, our_object_id);
			}

			return static_cast<tz::hanval>(our_object_id);
		}

//--------------------------------------------------------------------------------------------------

		const object_data& render_pass::get_object(object_handle oh) const
		{
			auto hanval = static_cast<std::size_t>(static_cast<tz::hanval>(oh));
			tz::assert(hanval < this->compute.get_draw_count());
			return this->obj.get_object_internals(this->render)[hanval];
		}

//--------------------------------------------------------------------------------------------------

		object_data& render_pass::get_object(object_handle oh)
		{
			auto hanval = static_cast<std::size_t>(static_cast<tz::hanval>(oh));
			tz::assert(hanval < this->compute.get_draw_count());
			return this->obj.get_object_internals(this->render)[hanval];
		}

//--------------------------------------------------------------------------------------------------

		void render_pass::remove_object(render_pass::object_handle oh)
		{
			TZ_PROFZONE("render_pass - remove object", 0xFFF1F474);
			auto hanval = static_cast<std::size_t>(static_cast<tz::hanval>(oh));
			this->compute.remove_draw(hanval);
			this->get_object(oh) = object_data{};

			auto maybe_node = this->tree.find_node(hanval);
			if(maybe_node.has_value())
			{
				auto this_node = this->tree.get_node(maybe_node.value());
				// remove this node from the hierarchy
				this->tree.remove_node(maybe_node.value(), transform_hierarchy<std::uint32_t>::remove_strategy::impl_do_nothing);
				// kill remove all its child objects too.
				for(unsigned int child_id : this_node.children)
				{
					auto child = this->tree.get_node(child_id);
					this->remove_object(static_cast<tz::hanval>(child.data));
				}
			}
		}

//--------------------------------------------------------------------------------------------------

		tz::trs render_pass::object_get_local_transform(object_handle oh) const
		{
			TZ_PROFZONE("render_pass - object get local transform", 0xFFF1F474);
			auto hanval = static_cast<std::size_t>(static_cast<tz::hanval>(oh));
			auto maybe_node = this->tree.find_node(hanval);
			tz::assert(maybe_node.has_value());
			return this->tree.get_node(maybe_node.value()).local_transform;
		}

//--------------------------------------------------------------------------------------------------

		void render_pass::object_set_local_transform(object_handle oh, tz::trs trs)
		{

			TZ_PROFZONE("render_pass - object set local transform", 0xFFF1F474);
			auto hanval = static_cast<std::size_t>(static_cast<tz::hanval>(oh));
			auto maybe_node = this->tree.find_node(hanval);
			tz::assert(maybe_node.has_value());
			this->tree.get_node(maybe_node.value()).local_transform = trs;
		}

//--------------------------------------------------------------------------------------------------

		tz::trs render_pass::object_get_global_transform(object_handle oh) const
		{
			TZ_PROFZONE("render_pass - object get global transform", 0xFFF1F474);
			auto hanval = static_cast<std::size_t>(static_cast<tz::hanval>(oh));
			auto maybe_node = this->tree.find_node(hanval);
			tz::assert(maybe_node.has_value());
			return this->tree.get_global_transform(maybe_node.value());
		}

//--------------------------------------------------------------------------------------------------

		void render_pass::object_set_global_transform(object_handle oh, tz::trs trs)
		{
			TZ_PROFZONE("render_pass - object set global transform", 0xFFF1F474);
			auto hanval = static_cast<std::size_t>(static_cast<tz::hanval>(oh));
			auto maybe_node = this->tree.find_node(hanval);
			tz::assert(maybe_node.has_value());
			const auto& node = this->tree.get_node(maybe_node.value());
			// convert global transform into local transform
			if(node.parent.has_value())
			{
				tz::trs parent_global = this->tree.get_global_transform(node.parent.value());
				parent_global.inverse();
				trs.combine(parent_global);
			}
			// then set the local transform.
			this->object_set_local_transform(oh, trs);
		}

//--------------------------------------------------------------------------------------------------

		texture_locator render_pass::object_get_texture(object_handle oh, std::size_t bound_texture_id) const
		{
			auto hanval = static_cast<std::size_t>(static_cast<tz::hanval>(oh));
			return obj.get_object_internals(this->render)[hanval].bound_textures[bound_texture_id];
		}

//--------------------------------------------------------------------------------------------------

		void render_pass::object_set_texture(object_handle oh, std::size_t bound_texture_id, texture_locator tloc)
		{
			auto hanval = static_cast<std::size_t>(static_cast<tz::hanval>(oh));
			obj.get_object_internals(this->render)[hanval].bound_textures[bound_texture_id] = tloc;
		}

//--------------------------------------------------------------------------------------------------

		bool render_pass::object_get_visible(object_handle oh) const
		{
			return this->compute.get_visibility_at(static_cast<std::size_t>(static_cast<tz::hanval>(oh)));
		}

//--------------------------------------------------------------------------------------------------

		void render_pass::object_set_visible(object_handle oh, bool visible)
		{
			this->compute.set_visibility_at(static_cast<std::size_t>(static_cast<tz::hanval>(oh)), visible);
		}

//--------------------------------------------------------------------------------------------------

		const tz::transform_hierarchy<std::uint32_t>& render_pass::get_hierarchy() const
		{
			return this->tree;
		}

//--------------------------------------------------------------------------------------------------

		tz::transform_hierarchy<std::uint32_t>& render_pass::get_hierarchy()
		{
			return this->tree;
		}

//--------------------------------------------------------------------------------------------------

		tz::gl::resource_handle render_pass::get_extra_buffer(std::size_t extra_buffer_id) const
		{
			tz::assert(this->extra_buf_hanval_first.has_value(), "Attempted to retrieve extra buffer %zu, but there were no extra buffers specified.", extra_buffer_id);
			tz::assert(extra_buffer_id < this->get_extra_buffer_count(), "Attempted to retrieve extra buffer %zu, but there were only %zu extra buffers specified", extra_buffer_id, this->get_extra_buffer_count());
			return static_cast<tz::hanval>(this->extra_buf_hanval_first.value() + extra_buffer_id);
		}

//--------------------------------------------------------------------------------------------------

		std::size_t render_pass::get_extra_buffer_count() const
		{
			if(this->extra_buf_hanval_first.has_value() && this->extra_buf_hanval_last.has_value())
			{
				// remember, if they're the same value then theres one buffer.
				// if either are nullopt, then there's none.
				return 1u + this->extra_buf_hanval_last.value() - this->extra_buf_hanval_first.value();
			}
			return 0u;
		}

//--------------------------------------------------------------------------------------------------

		tz::trs render_pass::get_camera_transform() const
		{
			return tz::trs::from_matrix(tz::gl::get_device().get_renderer(this->render).get_resource(this->camera_buffer)->data_as<tz::mat4>().front()).inverse();
		}

//--------------------------------------------------------------------------------------------------

		void render_pass::set_camera_transform(tz::trs camera_transform)
		{
			tz::gl::get_device().get_renderer(this->render).get_resource(this->camera_buffer)->data_as<tz::mat4>().front() = camera_transform.matrix().inverse();
		}

//--------------------------------------------------------------------------------------------------

		void render_pass::camera_perspective(camera_perspective_t persp)
		{
			tz::mat4 proj = tz::perspective(persp.fov, persp.aspect_ratio, persp.near_clip, persp.far_clip);
			tz::gl::get_device().get_renderer(this->render).get_resource(this->camera_buffer)->data_as<tz::mat4>().back() = proj;
		}

//--------------------------------------------------------------------------------------------------

		void render_pass::camera_orthographic(camera_orthographic_t ortho)
		{
			tz::mat4 proj = tz::orthographic(ortho.left, ortho.right, ortho.top, ortho.bottom, ortho.near_plane, ortho.far_plane);
			tz::gl::get_device().get_renderer(this->render).get_resource(this->camera_buffer)->data_as<tz::mat4>().back() = proj;
		}

//--------------------------------------------------------------------------------------------------

		tz::gl::renderer_handle render_pass::get_compute_pass() const
		{
			return this->compute.get_compute_pass();
		}

//--------------------------------------------------------------------------------------------------

		tz::gl::renderer_handle render_pass::get_render_pass() const
		{
			return this->render;
		}

//--------------------------------------------------------------------------------------------------

		bool render_pass::object_is_in_free_list(object_handle oh) const
		{
			return this->compute.is_in_free_list(static_cast<std::size_t>(static_cast<tz::hanval>(oh)));
		}

//--------------------------------------------------------------------------------------------------

	}	

//--------------------------------------------------------------------------------------------------
// mesh_renderer
//--------------------------------------------------------------------------------------------------

	mesh_renderer::mesh_renderer(mesh_renderer::info info):
	render_pass(info)
	{

	}

}