#include "tz/io/gltf.hpp"
#include "tz/core/debug.hpp"
#include <regex>

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

	std::span<const std::byte> gltf::view_buffer(gltf_buffer_view view) const
	{
		tz::assert(view.buffer_id == 0, "only one buffer is supported, otherwise it implies external bin data files, which topaz doesn't support.");
		return this->get_binary_data(view.offset, view.length);
	}

	std::span<const gltf_mesh> gltf::get_meshes() const
	{
		return this->meshes;
	}

	gltf::gltf(std::string_view glb_data)
	{
		this->parse_header(glb_data.substr(0, 12));
		this->parse_chunks(glb_data.substr(12));
		this->load_resources();
		this->create_views();
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
		if(chunkdata.empty())
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

	void gltf::create_views()
	{
		json bufviews = this->data["bufferViews"];
		if(bufviews.is_array())
		{
			for(auto view : bufviews)
			{
				tz::assert(view["buffer"] == 0);
				this->views.push_back
				({
					.buffer_id = 0,
		 			.type = static_cast<gltf_buffer_view_type>(view["target"]),
		 			.offset = view["byteOffset"],
		 			.length = view["byteLength"]
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
		ret.name = node["name"];
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
			if(!prim["material"].is_null())
			{
				tz::report("detected gltf mesh with one or more materials associated with it. materials are NYI.");
			}
			submesh.attributes = attribs;
			submesh.material_accessor = -1;
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
