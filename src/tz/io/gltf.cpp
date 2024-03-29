#include "tz/io/gltf.hpp"
#include "tz/core/debug.hpp"
#include "tz/core/profile.hpp"
#include "tz/core/matrix_transform.hpp"
#include "nlohmann/json.hpp"
#include <regex>
#include <fstream>
#undef assert

namespace tz::io
{
	using json = nlohmann::json;
	/**
	 * here's an example of what a glb's json chunk might look like:
		{
		"asset":
		{
			"generator":"Khronos glTF Blender I/O v3.6.27",
			"version":"2.0"
		},
		"scene":0,
		"scenes":
		[{
			"name":"Scene",
			"nodes":[0]
		}],
		"nodes":
		[{
			"mesh":0,
			"name":"Cube"
		}],
		"materials":
		[{
			"doubleSided":true,
			"name":"Material",
			"pbrMetallicRoughness":
			{
				"baseColorFactor":[0.800000011920929,0.800000011920929,0.800000011920929,1],
				"metallicFactor":0,
				"roughnessFactor":0.5
			}
		}],
		"meshes":
		[{
			"name":"Cube",
			"primitives":
			[{
				"attributes":{"POSITION":0,"NORMAL":1,"TEXCOORD_0":2},
				"indices":3,
				"material":0
			}]
		}],
		"accessors":
		[
			{"bufferView":0,"componentType":5126,"count":24,"max":[1,1,1],"min":[-1,-1,-1],"type":"VEC3"},
			{"bufferView":1,"componentType":5126,"count":24,"type":"VEC3"},
			{"bufferView":2,"componentType":5126,"count":24,"type":"VEC2"},
			{"bufferView":3,"componentType":5123,"count":36,"type":"SCALAR"}
		],
		"bufferViews":
		[
			{"buffer":0,"byteLength":288,"byteOffset":0,"target":34962},
			{"buffer":0,"byteLength":288,"byteOffset":288,"target":34962},
			{"buffer":0,"byteLength":192,"byteOffset":576,"target":34962},
			{"buffer":0,"byteLength":72,"byteOffset":768,"target":34963}
		],
		"buffers":
		[{
			"byteLength":840
		}]

		- looks like the glb defines a set of scenes, but chooses one of them.
		- scene contains nodes. nodes appear to be a set of meshes.
		- meshes contain primitives, which contain attributes.
		- attributes and indices are defined as references to accessors.
		- accessors seem to define essentially data spans of various types. in this case just buffer views.
		- buffer views show spans within buffers (which live in bin chunks not shown in this example).
	 */

	gltf gltf::from_memory(std::string_view sv)
	{
		TZ_PROFZONE("gltf - from memory", 0xFFFF2222);
		return {sv};
	}

	gltf gltf::from_file(const char* path)
	{
		TZ_PROFZONE("gltf - from file", 0xFFFF2222);
		std::ifstream file(path, std::ios::binary);
		tz::assert(file.good(), "Could not load gltf from file because the path was wrong, or something else went wrong.");
		std::string buffer(std::istreambuf_iterator<char>(file), {});
		return gltf::from_memory(buffer);
	}

	std::size_t gltf::vertex_count() const
	{
		std::size_t ret = 0;
		for(std::size_t i = 0; i < this->get_meshes().size(); i++)
		{
			for(std::size_t j = 0; j < this->get_meshes()[i].submeshes.size(); j++)
			{
				const auto& submesh = this->get_meshes()[i].submeshes[j];
				std::size_t posid = submesh.accessors[(int)gltf_attribute::position];
				ret += this->accessors[posid].element_count;
			}
		}
		return ret;
	}

	std::size_t gltf::index_count() const
	{
		std::size_t ret = 0;
		for(std::size_t i = 0; i < this->get_meshes().size(); i++)
		{
			for(std::size_t j = 0; j < this->get_meshes()[i].submeshes.size(); j++)
			{
				const auto& submesh = this->get_meshes()[i].submeshes[j];
				ret += this->accessors[submesh.indices_accessor].element_count;
			}
		}
		return ret;
	}

	std::span<const std::byte> gltf::view_buffer(gltf_buffer_view view) const
	{
		tz::assert(view.buffer_id == 0, "only one buffer is supported, otherwise it implies external bin data files, which topaz doesn't support.");
		return this->get_binary_data(view.offset, view.length);
	}

	std::span<const gltf_mesh> gltf::get_meshes() const
	{
		return this->meshes;
	}

	std::span<const gltf_buffer> gltf::get_buffers() const
	{
		return this->buffers;
	}

	std::span<const gltf_image> gltf::get_images() const
	{
		return this->images;
	}

	std::span<const gltf_material> gltf::get_materials() const
	{
		return this->materials;
	}

	std::span<const gltf_skin> gltf::get_skins() const
	{
		return this->skins;
	}

	std::span<const gltf_animation> gltf::get_animations() const
	{
		return this->animations;
	}

	std::span<const gltf_node> gltf::get_nodes() const
	{
		return this->nodes;
	}

	std::vector<gltf_node> gltf::get_root_nodes() const
	{
		std::vector<gltf_node> ret = {};
		for(std::size_t i = 0; i < this->nodes.size(); i++)
		{
			bool is_root_node = true;
			for(const auto& node : this->get_nodes())
			{
				auto iter = std::find(node.children.begin(), node.children.end(), i);
				if(iter != node.children.end())
				{
					// `i` is someone's child. isn't a root node.
					is_root_node = false;
					break;
				}
			}
			if(is_root_node)
			{
				ret.push_back(this->nodes[i]);
			}
		}
		return ret;
	}
	
	std::vector<gltf_node> gltf::get_active_nodes() const
	{
		if(this->active_scene_id == detail::badzu)
		{
			return {};
		}
		std::vector<gltf_node> top_level_nodes = {};
		const gltf_scene& scene = this->scenes[this->active_scene_id];
		for(std::size_t node_id : scene.nodes)
		{
			top_level_nodes.push_back(this->nodes[node_id]);
		}
		return top_level_nodes;
	}

	gltf_submesh_data gltf::get_submesh_vertex_data(std::size_t meshid, std::size_t submeshid) const
	{
		TZ_PROFZONE("gltf - retrieve submesh", 0xFFFF2222);
		gltf_submesh_data ret;
		tz::assert(this->get_meshes().size() > meshid, "Invalid meshid.");
		gltf_mesh mesh = this->get_meshes()[meshid];
		tz::assert(mesh.submeshes.size() > submeshid, "Invalid submeshid");
		gltf_mesh::submesh submesh = mesh.submeshes[submeshid];
		ret.name = mesh.name + ":" + std::to_string(submeshid + 1) + "/" + std::to_string(mesh.submeshes.size());
		ret.attributes = submesh.attributes;
		// we have a set of references to the positions, texcoords etc.
		// we need to interleave them into singular vertex data.
		// first we figure out how many vertices there are gonna be.
		// 1. retrieve the data spans for each vertex attribute
		// 2. iterate through each vertex and copy relevant snippets from the span into the resultant data. this requires manual interleaving.
		// 3. finally, copy over the index data.
		std::size_t vertex_count = 0;

		// 1
		// POS
		std::size_t posid = submesh.accessors[(int)gltf_attribute::position];
		tz::assert(posid != static_cast<std::size_t>(-1), "Detected submesh that was missing a position attribute. This cannmot be.");
		gltf_accessor pos = this->accessors[posid];
		tz::assert(pos.component_type == gltf_accessor_component_type::flt, "Position attribute was expected to be floats, but it is not. Please re-export the gltf with the expected config.");
		tz::assert(pos.type == gltf_accessor_type::vec3, "Position attributes should be vec3s. Please re-export with the expected config");
		vertex_count = pos.element_count;

		// NORMAL
		std::size_t nrmid = submesh.accessors[(int)gltf_attribute::normal];
		// TODO: meshes missing normals should be allowed.
		tz::assert(nrmid != static_cast<std::size_t>(-1));
		gltf_accessor nrm = this->accessors[nrmid];
		tz::assert(nrm.component_type == gltf_accessor_component_type::flt, "Normal attribute was expected to be floats, but it is not.");
		tz::assert(nrm.type == gltf_accessor_type::vec3, "Normals should be vec3.");
		tz::assert(vertex_count == nrm.element_count, "Number of normals did not match number of positions.");

		// TEXCOORDN
		std::array<std::size_t, gltf_max_texcoord_attribs> texcids;
		std::array<std::optional<gltf_accessor>, gltf_max_texcoord_attribs> texcs;
		for(std::size_t i = 0; i < gltf_max_texcoord_attribs; i++)
		{
			texcids[i] = submesh.accessors[(int)gltf_attribute::texcoord0 + i];
			if(texcids[i] != static_cast<std::size_t>(-1))
			{
				texcs[i] = this->accessors[texcids[i]];
				tz::assert(texcs[i]->component_type == gltf_accessor_component_type::flt, "Texcoord attribute was expected to be floats.");
				tz::assert(texcs[i]->type == gltf_accessor_type::vec2, "Texcoords expected to be vec2.");
				tz::assert(texcs[i]->element_count == vertex_count, "Number of texcoords did not match number of positions.");
			}
			else
			{
				texcs[i] = std::nullopt;
			}
		}

		// COLORN
		std::array<std::size_t, gltf_max_color_attribs> colorids;
		std::array<std::optional<gltf_accessor>, gltf_max_color_attribs> colors;
		for(std::size_t i = 0; i < gltf_max_color_attribs; i++)
		{
			colorids[i] = submesh.accessors[(int)gltf_attribute::color0 + i];
			if(colorids[i] != static_cast<std::size_t>(-1))
			{
				colors[i] = this->accessors[colorids[i]];
				tz::assert(colors[i]->component_type == gltf_accessor_component_type::flt, "Color attribute was expected to be floats.");
				tz::assert(colors[i]->type == gltf_accessor_type::vec3, "Colors expected to be vec2.");
				tz::assert(colors[i]->element_count == vertex_count, "Number of colors did not match number of positions.");
			}
			else
			{
				colors[i] = std::nullopt;
			}
		}

		// JOINTN
		std::array<std::size_t, gltf_max_joint_attribs> jointids;
		std::array<std::optional<gltf_accessor>, gltf_max_joint_attribs> joints;
		for(std::size_t i = 0; i < gltf_max_joint_attribs; i++)
		{
			jointids[i] = submesh.accessors[(int)gltf_attribute::joint0 + i];
			if(jointids[i] != detail::badzu)
			{
				joints[i] = this->accessors[jointids[i]];
				//tz::assert(joints[i]->component_type == gltf_accessor_component_type::ushort, "Joint attribute was expected to be unsigned shorts.");
				// topaz uses unsigned shorts natively, but will convert under-the-hood if the gltf does not use unsigned shorts.
				// note: gltf only supports unsigned bytes aswell as shorts.
				tz::assert(joints[i]->type == gltf_accessor_type::vec4, "Joints expected to be vec4.");
				tz::assert(joints[i]->element_count == vertex_count, "Number of joints did not match number of positions.");
			}
			else
			{
				joints[i] = std::nullopt;
			}
		}

		// WEIGHTN
		std::array<std::size_t, gltf_max_weight_attribs> weightids;
		std::array<std::optional<gltf_accessor>, gltf_max_weight_attribs> weights;
		for(std::size_t i = 0; i < gltf_max_weight_attribs; i++)
		{
			weightids[i] = submesh.accessors[(int)gltf_attribute::weight0 + i];
			if(weightids[i] != detail::badzu)
			{
				weights[i] = this->accessors[weightids[i]];
				tz::assert(weights[i]->component_type == gltf_accessor_component_type::flt, "Weight attribute was expected to be float.");
				tz::assert(weights[i]->type == gltf_accessor_type::vec4, "Weights expected to be vec4.");
				tz::assert(weights[i]->element_count == vertex_count, "Number of weight did not match number of positions.");
			}
		}

		// 2
		ret.vertices.reserve(vertex_count);

		// POS
		gltf_buffer_view pos_view = this->views[pos.buffer_view_id];
		std::span<const std::byte> pos_data = this->get_binary_data(pos_view.offset, pos_view.length);
		pos_data = pos_data.subspan(pos.byte_offset);

		// NORMAL
		gltf_buffer_view nrm_view = this->views[nrm.buffer_view_id];
		std::span<const std::byte> nrm_data = this->get_binary_data(nrm_view.offset, nrm_view.length);
		nrm_data = nrm_data.subspan(nrm.byte_offset);

		// TEXCOORN
		std::array<std::span<const std::byte>, gltf_max_texcoord_attribs> texc_datas;
		for(std::size_t i = 0; i < gltf_max_texcoord_attribs; i++)
		{
			if(texcs[i].has_value())
			{
				gltf_buffer_view texc_view = this->views[texcs[i]->buffer_view_id];
				texc_datas[i] = this->get_binary_data(texc_view.offset, texc_view.length);
				texc_datas[i] = texc_datas[i].subspan(texcs[i]->byte_offset);
			}
		}

		// COLORN
		std::array<std::span<const std::byte>, gltf_max_color_attribs> color_datas;
		for(std::size_t i = 0; i < gltf_max_color_attribs; i++)
		{
			if(colors[i].has_value())
			{
				gltf_buffer_view color_view = this->views[colors[i]->buffer_view_id];
				color_datas[i] = this->get_binary_data(color_view.offset, color_view.length);
				color_datas[i] = color_datas[i].subspan(colors[i]->byte_offset);
			}
		}

		// JOINTN
		std::array<std::span<const std::byte>, gltf_max_joint_attribs> joint_datas;
		for(std::size_t i = 0; i < gltf_max_joint_attribs; i++)
		{
			if(joints[i].has_value())
			{
				gltf_buffer_view joint_view = this->views[joints[i]->buffer_view_id];
				joint_datas[i] = this->get_binary_data(joint_view.offset, joint_view.length);
				joint_datas[i] = joint_datas[i].subspan(joints[i]->byte_offset);
			}
		}

		// WEIGHTN
		std::array<std::span<const std::byte>, gltf_max_weight_attribs> weight_datas;
		for(std::size_t i = 0; i < gltf_max_weight_attribs; i++)
		{
			if(weights[i].has_value())
			{
				gltf_buffer_view weight_view = this->views[weights[i]->buffer_view_id];
				weight_datas[i] = this->get_binary_data(weight_view.offset, weight_view.length);
				weight_datas[i] = weight_datas[i].subspan(weights[i]->byte_offset);
			}
		}

		constexpr std::size_t vec4_stride = sizeof(float) * 4;
		constexpr std::size_t vec3_stride = sizeof(float) * 3;
		constexpr std::size_t vec2_stride = sizeof(float) * 2;
		#define EVIL_TRANSFORM(in_span, out_container, type_to, type_from, count) do{auto* casted_ptr = reinterpret_cast<const type_from*>(in_span.data()); std::span<const type_from> typedata{casted_ptr, count}; std::transform(typedata.begin(), typedata.end(), out_container.begin(), [](const type_from& d)->type_to{return static_cast<type_to>(d);});}while(false)
		for(std::size_t i = 0; i < vertex_count; i++)
		{
			gltf_vertex_data& vtx = ret.vertices.emplace_back();
			
			std::memcpy(vtx.position.data().data(), pos_data.data() + (vec3_stride * i), sizeof(float) * 3);
			std::memcpy(vtx.normal.data().data(), nrm_data.data() + (vec3_stride * i), sizeof(float) * 3);
			for(std::size_t j = 0; j < gltf_max_texcoord_attribs; j++)
			{
				if(texcs[j].has_value())
				{
					std::memcpy(vtx.texcoordn[j].data().data(), texc_datas[j].data() + (vec2_stride * i), sizeof(float) * 2);
				}
			}
			for(std::size_t j = 0; j < gltf_max_color_attribs; j++)
			{
				if(colors[j].has_value())
				{
					std::memcpy(vtx.colorn[j].data().data(), color_datas[j].data() + (vec3_stride * i), sizeof(float) * 3);
				}
			}

			for(std::size_t j = 0; j < gltf_max_joint_attribs; j++)
			{
				std::vector<tz::vec4us> converted_joints;
				if(joints[j].has_value())
				{
					converted_joints.resize(joints[j]->element_count);
					switch(joints[j]->component_type)
					{
						case gltf_accessor_component_type::ubyte:
						{
							using vec4ub = std::array<std::byte, 4>;
							auto initial_joints = std::span<const vec4ub>{reinterpret_cast<const vec4ub*>(joint_datas[j].data()), joints[j]->element_count};
							std::transform(initial_joints.begin(), initial_joints.end(), converted_joints.begin(),
							[](vec4ub vec)
							{
								return tz::vec4us
								{
									static_cast<unsigned short>(vec[0]),
									static_cast<unsigned short>(vec[1]),
									static_cast<unsigned short>(vec[2]),
									static_cast<unsigned short>(vec[3])
								};
							});
						}
						break;
						case gltf_accessor_component_type::ushort:
							std::memcpy(converted_joints.data(), joint_datas[j].data(), converted_joints.size() * sizeof(tz::vec4us));
						break;
						default:

						break;
					}
					std::memcpy(vtx.jointn[j].data().data(), converted_joints.data() + i, sizeof(unsigned short) * 4);
				}
			}

			for(std::size_t j = 0; j < gltf_max_weight_attribs; j++)
			{
				if(weights[j].has_value())
				{
					std::memcpy(vtx.weightn[j].data().data(), weight_datas[j].data() + (vec4_stride * i), sizeof(float) * 4);
				}
			}
		}

		// finally, do indices.
		gltf_accessor indices = this->accessors[submesh.indices_accessor];
		std::size_t index_count = indices.element_count;
		tz::assert(indices.type == gltf_accessor_type::scalar, "indices should be scalars. not sure whats going on there.");
		gltf_buffer_view indices_view = this->views[indices.buffer_view_id];

		std::span<const std::byte> indices_data = this->get_binary_data(indices_view.offset, indices_view.length);
		indices_data = indices_data.subspan(indices.byte_offset);
		ret.indices.resize(index_count);
		std::vector<unsigned short> indices_intermediate;
		indices_intermediate.resize(index_count);

		std::memcpy(indices_intermediate.data(), indices_data.data(), sizeof(unsigned short) * index_count);
		#define EVIL_DO_TRANSFORM_BASED_ON(type) EVIL_TRANSFORM(indices_data, ret.indices, std::uint32_t, type, index_count)
		//EVIL_DO_TRANSFORM_BASED_ON(unsigned short);
		switch(indices.component_type)
		{
			case gltf_accessor_component_type::sbyte:
				EVIL_DO_TRANSFORM_BASED_ON(signed char);
			break;
			case gltf_accessor_component_type::ubyte:
				EVIL_DO_TRANSFORM_BASED_ON(unsigned char);
			break;
			case gltf_accessor_component_type::sshort:
				EVIL_DO_TRANSFORM_BASED_ON(std::int16_t);	
			break;
			case gltf_accessor_component_type::ushort:
				EVIL_DO_TRANSFORM_BASED_ON(std::uint16_t);
			break;
			case gltf_accessor_component_type::uint:
				EVIL_DO_TRANSFORM_BASED_ON(std::uint32_t);
			break;
			case gltf_accessor_component_type::flt:
				EVIL_DO_TRANSFORM_BASED_ON(float);
			break;
			default:
				tz::error("unrecognised accessor component type");
			break;
		}

		std::vector<gltf_submesh_texture_data> bound_textures = {};
		if(submesh.material_id != static_cast<std::size_t>(-1))
		{
			const auto& material = this->materials[submesh.material_id];
			tz::assert(material.color_texture_id != static_cast<std::size_t>(-1), "Detected GLTF material without base color texture. This is illegal in Topaz.");
			// base colour (required)
			bound_textures.push_back
			({
				.texcoord_id = material.color_texcoord_id,
				.image_id = material.color_texture_id,
				.factor = material.color_factor,
				.type = gltf_submesh_texture_type::color
			});
			// normal (optional)
			if(material.normal_texture_id != static_cast<std::size_t>(-1))
			{
				bound_textures.push_back
				({
					.texcoord_id = material.normal_texcoord_id,
					.image_id = material.normal_texture_id,
		 			.extra_data = material.normal_scale,
		 			.type = gltf_submesh_texture_type::normal
				});
			}
			// occlusion (optional)
			if(material.occlusion_texture_id != static_cast<std::size_t>(-1))
			{
				bound_textures.push_back
				({
					.texcoord_id = material.occlusion_texcoord_id,
					.image_id = material.occlusion_texture_id,
		 			.extra_data = material.occlusion_strength,
		 			.type = gltf_submesh_texture_type::occlusion
				});
			}
			// emissive (optional)
			if(material.emissive_texture_id != static_cast<std::size_t>(-1))
			{
				bound_textures.push_back
				({
					.texcoord_id = material.emissive_texcoord_id,
					.image_id = material.emissive_texture_id,
					.factor = material.emissive_factor.with_more(1.0f),
		 			.type = gltf_submesh_texture_type::emissive
				});
			}
		}
		ret.textures = bound_textures;
		return ret;
	}

	tz::io::image gltf::get_image_data(std::size_t imageid) const
	{
		TZ_PROFZONE("gltf - get image data", 0xFFFF2222);
		tz::assert(imageid < this->images.size(), "Invalid imageid %zu. Should be less than %zu", imageid, this->images.size());
		gltf_image img = this->images[imageid];
		std::span<const std::byte> imgdata = this->view_buffer(this->views[img.buffer_view_id]);

		std::string_view imgdata_sv{reinterpret_cast<const char*>(imgdata.data()), imgdata.size_bytes()};
		return tz::io::image::load_from_memory(imgdata_sv);
	}

	gltf::gltf(std::string_view glb_data):
	json_data(nlohmann::json{})
	{
		TZ_PROFZONE("gltf - import", 0xFFFF2222);
		this->parse_header(glb_data.substr(0, 12));
		this->parse_chunks(glb_data.substr(12));
		auto& data = std::any_cast<nlohmann::json&>(this->json_data);
		if(data["scene"].is_number_integer())
		{
			this->active_scene_id = data["scene"];
		}
		this->create_scenes();
		this->create_nodes();
		this->create_buffers();
		this->create_images();
		this->create_skins();
		this->create_materials();
		this->create_animations();
		this->create_views();
		this->create_accessors();
		this->create_meshes();
		this->compute_inverse_bind_matrices();
		this->compute_animation_data();
	}

	void gltf::parse_header(std::string_view header)
	{
		TZ_PROFZONE("gltf - parse header", 0xFFFF2222);
		tz::assert(header.length() == 12, "invalid header length. expected 12, got %zu", header.length());
		tz::assert(header.starts_with("glTF"), "Invalid GLB file format. Header's first 4 bytes should be \"glTF\", but it is \"%s\"", std::string{header.substr(0, 4)}.c_str());
		header.remove_prefix(4);
		std::uint32_t version;
		std::memcpy(&version, header.data(), 4);
		header.remove_prefix(4);
		std::uint32_t total_length;
		std::memcpy(&total_length, header.data(), 4);

		this->header =
		{
			.version = static_cast<std::size_t>(version),
			.size = static_cast<std::size_t>(total_length)
		};
		tz::assert(this->header.version == 2, "only glTF2 is supported. this is glTF%zu", this->header.version);
	}

	void gltf::parse_chunks(std::string_view chunkdata)
	{
		TZ_PROFZONE("gltf - parse chunks", 0xFFFF2222);
		if(chunkdata.empty() || chunkdata == "\n")
		{
			return;
		}
		else if(chunkdata.size() <= 8)
		{
			tz::error("Detected chunk of size <=8, which means data size is 0. That doesn't make much sense. Memory corruption?");
			return;
		}
		
		// next 8 bytes are special. a chunk length and a chunk type.
		std::uint32_t length;
		std::memcpy(&length, chunkdata.data(), 4);
		gltf_chunk_type t;
		chunkdata.remove_prefix(4);
		if(chunkdata.starts_with("JSON"))
		{
			t = gltf_chunk_type::json;
		}
		else if(chunkdata.starts_with("BIN"))
		{
			t = gltf_chunk_type::bin;
		}
		else if(chunkdata.starts_with("EXT"))
		{
			t = gltf_chunk_type::ext;
			tz::report("Detected EXT chunk within .glb. No extensions are supported by topaz, so the result may look unexpected.");
		}
		else
		{
			tz::error("Unrecognised chunk type \"%s\". Must be JSON, BIN or EXT. Corruption?", std::string{chunkdata.substr(0, 4)}.c_str());
			t = {};
		}
		chunkdata.remove_prefix(4);
		std::vector<std::byte> chunkdata_buf;
		chunkdata_buf.resize(length);
		std::memcpy(chunkdata_buf.data(), chunkdata.data(), length);
		this->chunks.push_back
		({
			.type = t,
	   		.data = chunkdata_buf
		});
		if(t == gltf_chunk_type::json)
		{
			TZ_PROFZONE("JSON Parse", 0xFFFF2222);
			std::string json_str(chunkdata.data(), length);
			this->json_data = json::parse(json_str);
		}
		chunkdata.remove_prefix(length);
		this->parse_chunks(chunkdata);
	}

	void gltf::create_scenes()
	{
		TZ_PROFZONE("gltf - create scenes", 0xFFFF2222);
		auto& data = std::any_cast<nlohmann::json&>(this->json_data);
		json scenes = data["scenes"];
		tz::assert(scenes.is_array() || scenes.is_null());
		if(scenes.is_array())
		{
			for(auto jscene : scenes)
			{
				auto& scene = this->scenes.emplace_back();
				if(jscene["name"].is_string())
				{
					scene.name = jscene["name"];
				}
				if(jscene["nodes"].is_array())
				{
					for(std::size_t node_id : jscene["nodes"])
					{
						scene.nodes.push_back(node_id);
					}
				}
			}
		}
	}

	void gltf::create_nodes()
	{
		TZ_PROFZONE("gltf - create nodes", 0xFFFF2222);
		auto& data = std::any_cast<nlohmann::json&>(this->json_data);
		json nodes = data["nodes"];
		tz::assert(nodes.is_array() || nodes.is_null());
		if(nodes.is_array())
		{
			std::size_t i = 0;
			for(auto jnode : nodes)
			{
				auto& node = this->nodes.emplace_back();
				node.id = i++;
				tz::assert(jnode["camera"].is_null(), "Node camera is not supported.");
				tz::assert(jnode["children"].is_array() || jnode["children"].is_null());
				if(jnode["children"].is_array())
				{
					for(std::size_t jchild : jnode["children"])
					{
						node.children.push_back(jchild);
					}
				}
				if(jnode["skin"].is_number_integer())
				{
					node.skin = jnode["skin"];
				}
				if(jnode["matrix"].is_array())
				{
					tz::mat4 mat;
					for(std::size_t i = 0; i < 16; i++)
					{
						mat(i / 4, i % 4) = jnode["matrix"][i];
					}
					node.transform = tz::trs::from_matrix(mat);
				}
				else
				{
					if(jnode["translation"].is_array())
					{
						for(std::size_t i = 0; i < 3; i++)
						{
							node.transform.translate[i] = jnode["translation"][i];
						}
					}
					if(jnode["rotation"].is_array())
					{
						for(std::size_t i = 0; i < 4; i++)
						{
							node.transform.rotate[i] = jnode["rotation"][i];
						}
					}
					if(jnode["scale"].is_array())
					{
						for(std::size_t i = 0; i < 3; i++)
						{
							node.transform.scale[i] = jnode["scale"][i];
						}
					}
				}
				if(jnode["mesh"].is_number_integer())
				{
					node.mesh = jnode["mesh"];
				}
				if(jnode["name"].is_string())
				{
					node.name = jnode["name"];
				}
			}
		}
	}

	void gltf::create_buffers()
	{
		TZ_PROFZONE("gltf - create buffers", 0xFFFF2222);
		auto& data = std::any_cast<nlohmann::json&>(this->json_data);
		json buffers = data["buffers"];
		tz::assert(buffers.is_array() || buffers.is_null());

		if(buffers.is_array())
		{
			for(auto buf : buffers)
			{
				this->buffers.push_back(this->load_buffer(&buf));
			}
		}	
	}

	void gltf::create_animations()
	{
		TZ_PROFZONE("gltf - create animations", 0xFFFF2222);
		auto& data = std::any_cast<nlohmann::json&>(this->json_data);
		json janims = data["animations"];
		tz::assert(janims.is_array() || janims.is_null());
		if(janims.is_array())
		{
			for(auto janim : janims)
			{
				auto& anim = this->animations.emplace_back();
				if(janim["name"].is_string())
				{
					anim.name = janim["name"];
				}
				
				tz::assert(janim["samplers"].is_array());
				for(auto samp : janim["samplers"])
				{
					tz::assert(samp["interpolation"].is_string());
					std::string erp = samp["interpolation"];
					gltf_animation_key_interpolation interp = gltf_animation_key_interpolation::linear;
					if(erp == "LINEAR")
					{
						interp = gltf_animation_key_interpolation::linear;
					}
					else if(erp == "STEP")
					{
						interp = gltf_animation_key_interpolation::step;
					}
					else if(erp == "CUBICSPLINE")
					{
						interp = gltf_animation_key_interpolation::cubic_spline;
					}
					else
					{
						tz::error("Unexpected janimation sampler key interpolation \"%s\". Should be either \"LINEAR\", \"STEP\", or \"CUBICSPLINE\".", erp.c_str());
					}
					anim.samplers.push_back
					({
						.input = samp["input"],
		  				.output = samp["output"],
		  				.interpolation = interp
					});
				}

				tz::assert(janim["channels"].is_array());
				for(auto chan : janim["channels"])
				{
					auto tar = chan["target"];
					tz::assert(tar.is_object());
					// gltf2: When node isn’t defined, channel SHOULD be ignored 
					if(tar["node"].is_null())
					{
						continue;
					}
					gltf_animation_channel_target_path path = gltf_animation_channel_target_path::rotation;
					std::string path_str = tar["path"];
					if(path_str == "translation")
					{
						path = gltf_animation_channel_target_path::translation;
					}
					else if(path_str == "rotation")
					{
						path = gltf_animation_channel_target_path::rotation;
					}
					else if(path_str == "scale")
					{
						path = gltf_animation_channel_target_path::scale;
					}
					else if(path_str == "weights")
					{
						path = gltf_animation_channel_target_path::weights;
					}
					else
					{
						tz::error("Unexpected animation channel target path \"%s\". Should be either \"translation\", \"rotation\", \"scale\", or \"weights\".", path_str.c_str());
					}
					anim.channels.push_back
					({
						.sampler_id = chan["sampler"],
		  				.target =
		  				{
							.node = static_cast<std::size_t>(tar["node"]),
			  				.path = path
						}
					});
				}
			}
		}
	}

	void gltf::create_images()
	{
		TZ_PROFZONE("gltf - load images", 0xFFFF2222);
		auto& data = std::any_cast<nlohmann::json&>(this->json_data);
		json imgs = data["images"];
		if(imgs.is_array())
		{
			for(auto img : imgs)
			{
				std::string img_type_str = img["mimeType"];
				gltf_image_type t;
				if(img_type_str == "image/png")
				{
					t = gltf_image_type::png;
				}
				else if(img_type_str == "image/jpeg")
				{
					t = gltf_image_type::jpg;
				}
				else
				{
					tz::error("Unexpected image mime type \"%s\". Only support jpg or png.", img_type_str.c_str());
					t = {};
				}
				std::string name = "Untitled";
				if(img["name"].is_string())
				{
					name = img["name"];
				}
				this->images.push_back
				({
		 			.name = name,
		 			.type = t,
		 			.buffer_view_id = img["bufferView"],
				});
			}
		}
	}

	void gltf::create_skins()
	{
		TZ_PROFZONE("gltf - create skins", 0xFFFF2222);
		auto& data = std::any_cast<nlohmann::json&>(this->json_data);
		json jskins = data["skins"];	
		if(jskins.is_array())
		{
			for(auto jskin : jskins)
			{
				auto& skin = this->skins.emplace_back();
				if(jskin["inverseBindMatrices"].is_number_integer())
				{
					skin.inverse_bind_matrix_accessor_id = jskin["inverseBindMatrices"];
				}
				if(jskin["skeleton"].is_number_integer())
				{
					skin.skeleton_id = jskin["skeleton"];
				}
				tz::assert(jskin["joints"].is_array(), "A skin must have a joints array according to the gltf spec. Malformed GLB.");
				for(std::size_t joint_id : jskin["joints"])
				{
					skin.joints.push_back(joint_id);
				}
				if(jskin["name"].is_string())
				{
					skin.name = jskin["name"];
				}
			}
		}
	}

	void gltf::create_materials()
	{
		TZ_PROFZONE("gltf - create materials", 0xFFFF2222);
		auto& data = std::any_cast<nlohmann::json&>(this->json_data);
		json mats = data["materials"];
		if(mats.is_array())
		{
			for(auto mat : mats)
			{
				tz::assert(mat["pbrMetallicRoughness"].is_object(), "Missing pbrMetallicRoughness param on material. GLB is in an invalid format.");
				auto pbr = mat["pbrMetallicRoughness"];
				auto normal = mat["normalTexture"];
				auto occlusion = mat["occlusionTexture"];
				auto emissive = mat["emissiveTexture"];
				if(!pbr["baseColorTexture"].is_object())
				{
					this->materials.push_back({});
					tz::assert(!normal.is_object(), "Detected GLTF material with no pbrmr base colour texture, but a normal map applied. In Topaz, normal maps require a base colour texture.");
					tz::assert(!occlusion.is_object(), "Detected GLTF material with no pbrmr base colour texture, but a occlusion map applied. In Topaz, occlusion maps require a base colour texture.");
					tz::assert(!emissive.is_object(), "Detected GLTF material with no pbrmr base colour texture, but an emissive map applied. In Topaz, emissive maps require a base colour texture.");
					continue;
				}
				std::size_t metallic_roughness_id = detail::badzu;
				std::size_t metallic_roughness_texcoord_id = 0;
				float metallic_factor = 1.0f;
				float roughness_factor = 1.0f;
				if(pbr["metallicRoughnessTexture"].is_object())
				{
					tz::assert(!pbr["metallicRoughnessTexture"]["index"].is_null(), "Missing pbrMetallicRoughness.metallicRoughnessTexture.index on material. GLB is in an invalid format.");
					metallic_roughness_id = pbr["metallicRoughnessTexture"]["index"];
					if(!pbr["metallicRoughnessTexture"]["texCoord"].is_null())
					{
						metallic_roughness_texcoord_id = pbr["metallicRoughnessTexture"]["texCoord"];
					}
				}
				if(!pbr["metallicFactor"].is_null())
				{
					metallic_factor = pbr["metallicFactor"];
				}
				if(!pbr["roughnessFactor"].is_null())
				{
					roughness_factor = pbr["roughnessFactor"];
				}
				tz::vec4 colour_factor = tz::vec4::filled(1.0f);
				if(!pbr["baseColorFactor"].is_null())
				{
					for(std::size_t i = 0; i < 4; i++)
					{
						colour_factor[i] = pbr["baseColorFactor"][i];
					}
				}
				tz::assert(!pbr["baseColorTexture"]["index"].is_null(), "Missing pbrMetallicRoughness.baseColorTexture.index on material. GLB is in an invalid format, or Topaz needs a patch to support this GLB.");
				std::size_t imgid = pbr["baseColorTexture"]["index"];
				std::size_t img_texcoord_id = 0;
				if(!pbr["baseColorTexture"]["texCoord"].is_null())
				{
					img_texcoord_id = pbr["baseColorTexture"]["texCoord"];
				}
				std::size_t normid = -1;
				std::size_t norm_coord_id = 0;
				float norm_scale = 1.0f;
				std::size_t occid = -1;
				std::size_t occ_coord_id = 0;
				float occ_strength = 1.0f;
				std::size_t emid = -1;
				std::size_t em_coord_id = 0;
				if(normal.is_object())
				{
					normid = normal["index"];
					if(!normal["texCoord"].is_null())
					{
						norm_coord_id = normal["texCoord"];
					}
					if(!normal["scale"].is_null())
					{
						norm_scale = normal["scale"];
					}
				}
				if(occlusion.is_object())
				{
					occid = occlusion["index"];
					if(!occlusion["texCoord"].is_null())
					{
						occ_coord_id = occlusion["texCoord"];
					}
					if(!occlusion["scale"].is_null())
					{
						occ_strength = occlusion["scale"];
					}
				}
				if(emissive.is_object())
				{
					emid = emissive["index"];
					if(!emissive["texCoord"].is_null())
					{
						em_coord_id = emissive["texCoord"];
					}
					tz::assert(emissive["scale"].is_null(), "`emissiveTexture.scale` is not supported. Please re-export GLTF without this property.");
				}
				std::string name = "Untitled";
				if(mat["name"].is_string())
				{
					name = mat["name"];
				}
				this->materials.push_back
				({
					.name = name,
		 			.color_texture_id = imgid,
		 			.color_texcoord_id = img_texcoord_id,
					.color_factor = colour_factor,
					.metallic_roughness_texture_id = metallic_roughness_id,
					.metallic_roughness_texcoord_id = metallic_roughness_texcoord_id,
					.metallic_factor = metallic_factor,
					.roughness_factor = roughness_factor,
		 			.normal_texture_id = normid,
		 			.normal_texcoord_id = norm_coord_id,
		 			.normal_scale = norm_scale,
		 			.occlusion_texture_id = occid,
		 			.occlusion_texcoord_id = occ_coord_id,
		 			.occlusion_strength = occ_strength,
		 			.emissive_texture_id = emid,
		 			.emissive_texcoord_id = em_coord_id
				});
			}
		}
	}

	void gltf::create_views()
	{
		TZ_PROFZONE("gltf - create views", 0xFFFF2222);
		auto& data = std::any_cast<nlohmann::json&>(this->json_data);
		json bufviews = data["bufferViews"];
		if(bufviews.is_array())
		{
			for(auto view : bufviews)
			{
				tz::assert(view["buffer"] == 0);
				gltf_buffer_view_type t;
				if(view["target"].is_null())
				{
					t = gltf_buffer_view_type::none;	
				}
				else
				{
					t = static_cast<gltf_buffer_view_type>(view["target"]);
				}
				// byteOffset is optional - defaults to 0.
				std::size_t offset = 0;
				if(view["byteOffset"].is_number())
				{
					offset = view["byteOffset"];
				}
				this->views.push_back
				({
					.buffer_id = 0,
		 			.type = t,
		 			.offset = offset,
		 			.length = view["byteLength"]
				});
			}
		}
	}

	void gltf::create_accessors()
	{
		TZ_PROFZONE("gltf - create accessors", 0xFFFF2222);
		auto& data = std::any_cast<nlohmann::json&>(this->json_data);
		json node = data["accessors"];
		if(node.is_array())
		{
			for(auto accessor : node)
			{
				tz::assert(accessor["sparse"].is_null());
				int byte_offset = 0;
				if(!accessor["byteOffset"].is_null())
				{
					byte_offset = accessor["byteOffset"];
				}
				std::string type = accessor["type"];
				gltf_accessor_type t = {};
				if(type == "SCALAR")
				{
					t = gltf_accessor_type::scalar;
				}
				else if(type == "VEC2")
				{
					t = gltf_accessor_type::vec2;
				}
				else if(type == "VEC3")
				{
					t = gltf_accessor_type::vec3;
				}
				else if(type == "VEC4")
				{
					t = gltf_accessor_type::vec4;
				}
				else if(type == "MAT2")
				{
					t = gltf_accessor_type::mat2;
				}
				else if(type == "MAT3")
				{
					t = gltf_accessor_type::mat3;
				}
				else if(type == "MAT4")
				{
					t = gltf_accessor_type::mat4;
				}
				else
				{
					tz::assert("Unrecognised accessor type \"%s\". Corrupted GLB file or too new and somehow got past topaz verification.", type.c_str());
				}
				auto comp_type = static_cast<gltf_accessor_component_type>(accessor["componentType"]);
				tz::mat4 max{};
				tz::mat4 min{};
				for(std::size_t i = 0; i < 4; i++)
				{
					for(std::size_t j = 0; j < 4; j++)
					{
						max(i, j) = std::numeric_limits<float>::max();
						min(i, j) = 0.0f;
					}
				}
				if(!accessor["max"].is_null())
				{
					if(accessor["max"].is_array())
					{
						std::size_t i = 0;
						for(auto val : accessor["max"])
						{
							max(i / 4, i % 4) = val;
							i++;
						}
					}
					else
					{
						max(0, 0) = accessor["max"];
					}
				}
				if(!accessor["min"].is_null())
				{
					if(accessor["min"].is_array())
					{
						std::size_t i = 0;
						for(auto val : accessor["min"])
						{
							min(i / 4, i % 4) = val;
							i++;
						}
					}
					else
					{
						min(0, 0) = accessor["min"];
					}
				}
				this->accessors.push_back
				({
					.buffer_view_id = accessor["bufferView"],
					.byte_offset = static_cast<std::size_t>(byte_offset),
		 			.component_type = comp_type,
		 			.element_count = accessor["count"],
		 			.type = t,
					.max = max,
					.min = min
				});
			}
		}
	}

	void gltf::create_meshes()
	{
		TZ_PROFZONE("gltf - create meshes", 0xFFFF2222);
		auto& data = std::any_cast<nlohmann::json&>(this->json_data);
		json meshes = data["meshes"];
		if(meshes.is_array())
		{
			for(auto mesh : meshes)
			{
				this->meshes.push_back(this->load_mesh(&mesh));
			}
		}
	}

	gltf_buffer gltf::load_buffer(void* nodeptr)
	{
		TZ_PROFZONE("gltf - load buffer", 0xFFFF2222);
		auto node = *static_cast<json*>(nodeptr);
		std::string makeshift_bufname = "buffer" + std::to_string(this->parsed_buf_count);
		tz::assert(node.contains("byteLength"), "gltf json buffer object \"%s\" does not have an entry named `byteLength`", makeshift_bufname.c_str());
		std::size_t byte_length = node["byteLength"];
		/*
		 * The glTF asset MAY use the GLB file container to pack glTF JSON and one glTF buffer into one file.
		 * Data for such a buffer is provided via the GLB-stored BIN chunk.
		 * A buffer with data provided by the GLB-stored BIN chunk, MUST be the first element of buffers
		 array and it MUST have its buffer.uri property undefined. When such a buffer exists, a BIN chunk
		 MUST be present.
		 * Any glTF buffer with undefined buffer.uri property that is not the first element of buffers array
		 does not refer to the GLB-stored BIN chunk, and the behavior of such buffers is left undefined to
		 accommodate future extensions and specification versions.
		 * The byte length of the BIN chunk MAY be up to 3 bytes bigger than JSON-defined buffer.byteLength
		 value to satisfy GLB padding requirements.
		*/
		// given this, we should actually only ever have one buffer?
		tz::assert(!node.contains("uri"), "buffer `uri` properties cannot be defined if the buffer data is within a binary chunk. as that's the only thing Topaz supports, seeing this `uri` property means the GLB is misconfigured.");
		this->parsed_buf_count++;
		return
		{
			.label = makeshift_bufname,
			.data = this->get_binary_data(0, byte_length)
		};	
	}

	gltf_mesh gltf::load_mesh(void* nodeptr)
	{
		TZ_PROFZONE("gltf - load mesh", 0xFFFF2222);
		auto node = *static_cast<json*>(nodeptr);
		gltf_mesh ret;
		ret.name = "Untitled";
		if(node["name"].is_string())
		{
			ret.name = node["name"];
		}
		tz::assert(node["primitives"].is_array(), "Mesh did not contain a valid primitives array. Topaz does not support empty gltf meshes.");
		for(auto prim : node["primitives"])
		{
			gltf_mesh::submesh& submesh = ret.submeshes.emplace_back();
			// some explanation is sorely needed here for future harry.
			// our submeshes could have various different attributes.
			// we need to know: which attributes this submesh has, and which buffer views represent the values for each attribute.
			// we also check for indices and materials.
			if(!prim["indices"].is_null())
			{
				// has an index buffer.
				submesh.indices_accessor = prim["indices"];
			}
			tz::assert(!prim["attributes"].is_null());
			gltf_attributes attribs = {};
			// let's figoure out which attributes this submesh has.
			// start by setting all the attribute accessor references to -1. this means any missing attributes that this submesh doesnt use will not only be missing from the enum field, but also retain a reference of -1.
			for(std::size_t i = 0; i < (int)gltf_attribute::_count; i++)
			{
				submesh.accessors[i] = -1;
			}
			// is there a position? if so, register it in the field and then patch the accessor reference.
			if(!prim["attributes"]["POSITION"].is_null())
			{
				attribs |= gltf_attribute::position;
				submesh.accessors[(int)gltf_attribute::position] = prim["attributes"]["POSITION"];
			}
			// same with the others!
			if(!prim["attributes"]["NORMAL"].is_null())
			{
				attribs |= gltf_attribute::normal;
				submesh.accessors[(int)gltf_attribute::normal] = prim["attributes"]["NORMAL"];
			}
			if(!prim["attributes"]["TANGENT"].is_null())
			{
				attribs |= gltf_attribute::tangent;
				submesh.accessors[(int)gltf_attribute::tangent] = prim["attributes"]["TANGENT"];
			}
			// as we can have multiple texcoords, colours, joints and weights, we iterate through all possibilities here.
			// texcoords
			for(int i = 0; i < gltf_max_texcoord_attribs; i++)
	  		{
				std::string keyname = "TEXCOORD_" + std::to_string(i);
				if(!prim["attributes"][keyname.c_str()].is_null())
				{
					attribs |= static_cast<gltf_attribute>((int)gltf_attribute::texcoord0 + i);
					submesh.accessors[(int)gltf_attribute::texcoord0 + i] = prim["attributes"][keyname.c_str()];
				}
				// in our topaz implementation, we have a hardcoded limit of these.
				// let's test if max+1 exists, if so we know we don't have enough to support this mesh.
			}
			std::string texcoord_large = "TEXCOORD_" + std::to_string((int)gltf_attribute::texcoord0 + gltf_max_texcoord_attribs);
			tz::assert(prim["attributes"][texcoord_large.c_str()].is_null(), "Detected GLTF attribute %s on a submesh, but we only support upto %d of those", texcoord_large.c_str(), gltf_max_texcoord_attribs);
			// and then the rest...
			// colours.
			for(int i = 0; i < gltf_max_color_attribs; i++)
	  		{
				std::string keyname = "COLOR_" + std::to_string(i);
				if(!prim["attributes"][keyname.c_str()].is_null())
				{
					attribs |= static_cast<gltf_attribute>((int)gltf_attribute::color0 + i);
					submesh.accessors[(int)gltf_attribute::color0 + i] = prim["attributes"][keyname.c_str()];
				}
			}
			std::string color_large = "COLOR_" + std::to_string((int)gltf_attribute::color0 + gltf_max_color_attribs);
			tz::assert(prim["attributes"][color_large.c_str()].is_null(), "Detected GLTF attribute %s on a submesh, but we only support upto %d of those", color_large.c_str(), gltf_max_color_attribs);
			// joints.
			for(int i = 0; i < gltf_max_joint_attribs; i++)
	  		{
				std::string keyname = "JOINTS_" + std::to_string(i);
				if(!prim["attributes"][keyname.c_str()].is_null())
				{
					attribs |= static_cast<gltf_attribute>((int)gltf_attribute::joint0 + i);
					submesh.accessors[(int)gltf_attribute::joint0 + i] = prim["attributes"][keyname.c_str()];
				}
			}
			std::string joint_large = "JOINTS_" + std::to_string((int)gltf_attribute::joint0 + gltf_max_joint_attribs);
			tz::assert(prim["attributes"][joint_large.c_str()].is_null(), "Detected GLTF attribute %s on a submesh, but we only support upto %d of those", joint_large.c_str(), gltf_max_joint_attribs);
			// weights.
			for(int i = 0; i < gltf_max_weight_attribs; i++)
	  		{
				std::string keyname = "WEIGHTS_" + std::to_string(i);
				if(!prim["attributes"][keyname.c_str()].is_null())
				{
					attribs |= static_cast<gltf_attribute>((int)gltf_attribute::weight0 + i);
					submesh.accessors[(int)gltf_attribute::weight0 + i] = prim["attributes"][keyname.c_str()];
				}
			}
			std::string weight_large = "WEIGHTS_" + std::to_string((int)gltf_attribute::weight0 + gltf_max_weight_attribs);
			tz::assert(prim["attributes"][weight_large.c_str()].is_null(), "Detected GLTF attribute %s on a submesh, but we only support upto %d of those", weight_large.c_str(), gltf_max_weight_attribs);
			
			submesh.material_id = -1;
			if(!prim["material"].is_null())
			{
				submesh.material_id = prim["material"];
			}
			submesh.attributes = attribs;
			if(prim["mode"].is_null())
			{
				submesh.mode = gltf_primitive_mode::triangles;
			}
			else
			{
				submesh.mode = static_cast<gltf_primitive_mode>(prim["mode"]);
				tz::assert(submesh.mode == gltf_primitive_mode::triangles, "Detected submesh that does not consist of triangle primitives. Only triangle primitives are allowed in Topaz. Please re-export your glb accordingly.");
			}
		}
		return ret;
	}

	std::span<const std::byte> gltf::get_binary_data(std::size_t offset, std::size_t len) const
	{
		// there must be a json chunk, but optionally a bin chunk.
		// if we're asked for binary data, then there must be a bin chunk.
		tz::assert(this->chunks.size() >= 2);
		auto iter = std::find_if(this->chunks.begin(), this->chunks.end(),
		[](const gltf_chunk_data& chunk)
		{
			return chunk.type == gltf_chunk_type::bin;
		});
		tz::assert(iter != this->chunks.end(), "Could not find a binary chunk within the GLB after being requested binary data.");
		return std::span<const std::byte>{iter->data}.subspan(offset, len);
	}

	void gltf::compute_inverse_bind_matrices()
	{
		for(auto& skin : this->skins)
		{
			skin.inverse_bind_matrices.resize(skin.joints.size(), tz::mat4::identity());
			if(skin.inverse_bind_matrix_accessor_id != detail::badzu)
			{
				gltf_accessor accessor = this->accessors[skin.inverse_bind_matrix_accessor_id];
				tz::assert(accessor.component_type == gltf_accessor_component_type::flt);
				tz::assert(accessor.type == gltf_accessor_type::mat4);
				tz::assert(accessor.buffer_view_id != detail::badzu);
				std::span<const std::byte> matrix_data = this->view_buffer(this->views[accessor.buffer_view_id]);
				matrix_data = matrix_data.subspan(accessor.byte_offset);
				std::span<const tz::mat4> matrices{reinterpret_cast<const tz::mat4*>(matrix_data.data()), accessor.element_count};
				tz::assert(matrices.size() == accessor.element_count);
				std::copy(matrices.begin(), matrices.end(), skin.inverse_bind_matrices.begin());
			}
		}
	}

	void gltf::compute_animation_data()
	{
		// retrieve the time points and transforms and populate gltf_animation::node_animation_data
		for(gltf_animation& anim : this->animations)
		{
			anim.node_animation_data.resize(this->get_nodes().size());

			for(const gltf_animation_channel& channel : anim.channels)
			{
				gltf_animation_sampler& sampler = anim.samplers[channel.sampler_id];
				const gltf_animation_channel_target& target = channel.target;

				const std::size_t node_id = target.node;
				auto& [kf_positions, kf_rotations, kf_scales] = anim.node_animation_data[node_id];
				const gltf_accessor& input_accessor = this->accessors[sampler.input];
				anim.max_time = std::max(input_accessor.max(0, 0), anim.max_time);
				const gltf_accessor& output_accessor = this->accessors[sampler.output];

				// do some validation.
				// inputs are expected to be an array of floats.
				tz::assert(input_accessor.component_type == gltf_accessor_component_type::flt);
				tz::assert(input_accessor.type == gltf_accessor_type::scalar);
				// outputs are expected to either be: an array of vec4s (if rotation), otherwise an array of vec3s. always floats.
				tz::assert(output_accessor.component_type == gltf_accessor_component_type::flt);
				tz::assert(target.path != gltf_animation_channel_target_path::weights, "Animation channel target path `weights` is not supported in Topaz.");
				if(target.path == gltf_animation_channel_target_path::rotation)
				{
					tz::assert(output_accessor.type == gltf_accessor_type::vec4);
				}
				else
				{
					tz::assert(target.path == gltf_animation_channel_target_path::translation || target.path == gltf_animation_channel_target_path::scale);
					tz::assert(output_accessor.type == gltf_accessor_type::vec3);
				}

				// once we validated everythings in the correct format, let's extract out the data.
				auto time_bytes = this->view_buffer(this->views[input_accessor.buffer_view_id]);
				time_bytes = time_bytes.subspan(input_accessor.byte_offset);
				auto transform_bytes = this->view_buffer(this->views[output_accessor.buffer_view_id]);
				transform_bytes = transform_bytes.subspan(output_accessor.byte_offset);

				const std::size_t keyframe_count = input_accessor.element_count;
				tz::assert(keyframe_count == output_accessor.element_count);
				std::span<const float> time_floats{reinterpret_cast<const float*>(time_bytes.data()), keyframe_count};

				if(target.path == gltf_animation_channel_target_path::rotation)
				{
					std::span<const tz::vec4> transform_vec4s{reinterpret_cast<const tz::vec4*>(transform_bytes.data()), keyframe_count};
					for(std::size_t i = 0; i < keyframe_count; i++)
					{
						tz::vec4 data = transform_vec4s[i];
						kf_rotations.insert({.time_point = time_floats[i], .transform = data});
					}
				}
				else
				{
					std::span<const tz::vec3> transform_vec3s{reinterpret_cast<const tz::vec3*>(transform_bytes.data()), keyframe_count};
					for(std::size_t i = 0; i < keyframe_count; i++)
					{
						tz::vec3 data = transform_vec3s[i];
						switch(target.path)
						{
							case gltf_animation_channel_target_path::translation:
								kf_positions.insert({.time_point = time_floats[i], .transform = data.with_more(0.0f)});
							break;
							case gltf_animation_channel_target_path::scale:
								kf_scales.insert({.time_point = time_floats[i], .transform = data.with_more(0.0f)});
							break;
							default:
								tz::error();
							break;
						}
					}
				}
			}
		}
	}
}
