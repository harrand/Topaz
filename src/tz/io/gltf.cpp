#include "tz/io/gltf.hpp"
#include "tz/core/debug.hpp"
#include <regex>
#include <fstream>

namespace tz::io
{
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
		return {sv};
	}

	gltf gltf::from_file(const char* path)
	{
		std::ifstream file(path, std::ios::binary);
		tz::assert(file.good(), "Could not load gltf from file because the path was wrong, or something else went wrong.");
		std::string buffer(std::istreambuf_iterator<char>(file), {});
		return gltf::from_memory(buffer);
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

	std::span<const gltf_image> gltf::get_images() const
	{
		return this->images;
	}

	gltf_submesh_data gltf::get_submesh_vertex_data(std::size_t meshid, std::size_t submeshid) const
	{
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

		// 2
		ret.vertices.reserve(vertex_count);

		// POS
		gltf_buffer_view pos_view = this->views[pos.buffer_view_id];
		std::span<const std::byte> pos_data = this->get_binary_data(pos_view.offset, pos_view.length);

		// NORMAL
		gltf_buffer_view nrm_view = this->views[nrm.buffer_view_id];
		std::span<const std::byte> nrm_data = this->get_binary_data(nrm_view.offset, nrm_view.length);

		// TEXCOORN
		std::array<std::span<const std::byte>, gltf_max_texcoord_attribs> texc_datas;
		for(std::size_t i = 0; i < gltf_max_texcoord_attribs; i++)
		{
			if(texcs[i].has_value())
			{
				gltf_buffer_view texc_view = this->views[texcs[i]->buffer_view_id];
				texc_datas[i] = this->get_binary_data(texc_view.offset, texc_view.length);
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
			}
		}

		constexpr std::size_t vec3_stride = sizeof(float) * 3;
		constexpr std::size_t vec2_stride = sizeof(float) * 2;
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
		}

		// finally, do indices.
		gltf_accessor indices = this->accessors[submesh.indices_accessor];
		std::size_t index_count = indices.element_count;
		tz::assert(indices.component_type == gltf_accessor_component_type::ushort);
		tz::assert(indices.type == gltf_accessor_type::scalar, "indices should be scalars. not sure whats going on there.");
		gltf_buffer_view indices_view = this->views[indices.buffer_view_id];
		std::span<const std::byte> indices_data = this->get_binary_data(indices_view.offset, indices_view.length);
		ret.indices.resize(index_count);
		std::vector<unsigned short> indices_intermediate;
		indices_intermediate.resize(index_count);

		std::memcpy(indices_intermediate.data(), indices_data.data(), sizeof(unsigned short) * index_count);
		std::transform(indices_intermediate.begin(), indices_intermediate.end(), ret.indices.begin(),
		[](unsigned short s) -> std::uint32_t
		{
			// just do implicit conversion.
			return s;
		});

		if(submesh.material_id != static_cast<std::size_t>(-1))
		{
			ret.bound_image_id = this->materials[submesh.material_id].pbr_metallic_roughness_base_color_texture_id;
		}
		return ret;
	}

	tz::io::image gltf::get_image_data(std::size_t imageid) const
	{
		tz::assert(imageid < this->images.size(), "Invalid imageid %zu. Should be less than %zu", imageid, this->images.size());
		gltf_image img = this->images[imageid];
		std::span<const std::byte> imgdata = this->view_buffer(this->views[img.buffer_view_id]);

		std::string_view imgdata_sv{reinterpret_cast<const char*>(imgdata.data()), imgdata.size_bytes()};
		return tz::io::image::load_from_memory(imgdata_sv);
	}

	gltf::gltf(std::string_view glb_data)
	{
		this->parse_header(glb_data.substr(0, 12));
		this->parse_chunks(glb_data.substr(12));
		this->load_resources();
		this->create_images();
		this->create_materials();
		this->create_views();
		this->create_accessors();
		this->create_meshes();
	}

	void gltf::parse_header(std::string_view header)
	{
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
			std::string json_str(chunkdata.data(), length);
			this->data = json::parse(json_str);
		}
		chunkdata.remove_prefix(length);
		this->parse_chunks(chunkdata);
	}

	void gltf::load_resources()
	{
		// according to spec, top level buffers and images contain lists of 'buffer' and 'image' respectively.
		json buffers = this->data["buffers"];
		tz::assert(buffers.is_array() || buffers.is_null());
		json images = this->data["images"];
		tz::assert(images.is_array() || images.is_null());

		if(buffers.is_array())
		{
			for(auto buf : buffers)
			{
				this->resources.push_back(this->load_buffer(buf));
			}
		}	
	}

	void gltf::create_images()
	{
		json imgs = this->data["images"];
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

	void gltf::create_materials()
	{
		json mats = this->data["materials"];
		if(mats.is_array())
		{
			for(auto mat : mats)
			{
				tz::assert(mat["pbrMetallicRoughness"].is_object(), "Missing pbrMetallicRoughness param on material. GLB is in an invalid format.");
				if(!mat["pbrMetallicRoughness"]["baseColorTexture"].is_object())
				{
					this->materials.push_back({});
					continue;
				}
				tz::assert(!mat["pbrMetallicRoughness"]["baseColorTexture"]["index"].is_null(), "Missing pbrMetallicRoughness.baseColorTexture.index on material. GLB is in an invalid format, or Topaz needs a patch to support this GLB.");
				std::size_t imgid = mat["pbrMetallicRoughness"]["baseColorTexture"]["index"];
				std::string name = "Untitled";
				if(mat["name"].is_string())
				{
					name = mat["name"];
				}
				this->materials.push_back
				({
					.name = name,
		 			.pbr_metallic_roughness_base_color_texture_id =	imgid
				});
			}
		}
	}

	void gltf::create_views()
	{
		json bufviews = this->data["bufferViews"];
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
				this->views.push_back
				({
					.buffer_id = 0,
		 			.type = t,
		 			.offset = view["byteOffset"],
		 			.length = view["byteLength"]
				});
			}
		}
	}

	void gltf::create_accessors()
	{
		json node = this->data["accessors"];
		if(node.is_array())
		{
			for(auto accessor : node)
			{
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
				this->accessors.push_back
				({
					.buffer_view_id = accessor["bufferView"],
		 			.component_type = static_cast<gltf_accessor_component_type>(accessor["componentType"]),
		 			.element_count = accessor["count"],
		 			.type = t
				});
			}
		}
	}

	void gltf::create_meshes()
	{
		json meshes = this->data["meshes"];
		if(meshes.is_array())
		{
			for(auto mesh : meshes)
			{
				this->meshes.push_back(this->load_mesh(mesh));
			}
		}
	}

	gltf_resource gltf::load_buffer(json node)
	{
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

	gltf_mesh gltf::load_mesh(json node)
	{
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
				std::string keyname = "JOINT_" + std::to_string(i);
				if(!prim["attributes"][keyname.c_str()].is_null())
				{
					attribs |= static_cast<gltf_attribute>((int)gltf_attribute::joint0 + i);
					submesh.accessors[(int)gltf_attribute::joint0 + i] = prim["attributes"][keyname.c_str()];
				}
			}
			std::string joint_large = "JOINT_" + std::to_string((int)gltf_attribute::joint0 + gltf_max_joint_attribs);
			tz::assert(prim["attributes"][joint_large.c_str()].is_null(), "Detected GLTF attribute %s on a submesh, but we only support upto %d of those", joint_large.c_str(), gltf_max_joint_attribs);
			// weights.
			for(int i = 0; i < gltf_max_weight_attribs; i++)
	  		{
				std::string keyname = "WEIGHT_" + std::to_string(i);
				if(!prim["attributes"][keyname.c_str()].is_null())
				{
					attribs |= static_cast<gltf_attribute>((int)gltf_attribute::weight0 + i);
					submesh.accessors[(int)gltf_attribute::weight0 + i] = prim["attributes"][keyname.c_str()];
				}
			}
			std::string weight_large = "WEIGHT_" + std::to_string((int)gltf_attribute::weight0 + gltf_max_weight_attribs);
			tz::assert(prim["attributes"][weight_large.c_str()].is_null(), "Detected GLTF attribute %s on a submesh, but we only support upto %d of those", weight_large.c_str(), gltf_max_weight_attribs);
			
			// materials not yet implemented.
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
}
