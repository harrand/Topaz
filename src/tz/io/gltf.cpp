#include "tz/io/gltf.hpp"
#include "tz/core/debug.hpp"
#include <regex>

namespace tz::io
{
	gltf gltf::from_memory(std::string_view sv)
	{
		return {json::parse(sv)};
	}

	gltf::gltf(json data):
	data(data)
	{
		this->parse_resources();
	}

	void gltf::parse_resources()
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

	gltf_resource gltf::load_buffer(json node)
	{
		std::string makeshift_bufname = "buffer" + std::to_string(this->parsed_buf_count);
		std::vector<std::byte> bufdata;
		tz::assert(node.contains("byteLength"), "gltf json buffer object \"%s\" does not have an entry named `byteLength`", makeshift_bufname.c_str());
		std::size_t byte_length = node["byteLength"];
		bufdata.resize(byte_length, std::byte{0});

		if(node.contains("uri"))
		{
			// we only support data uris.
			std::string uri = node["uri"];
			std::size_t new_sz = parse_data_uri(uri, bufdata);
			tz::assert(new_sz <= byte_length, "data uri size increased after being parsed (%zu -> %zu). it should only shrink due to base64 decoding or stay the same.", byte_length, new_sz);
			// note: data itself might be base64 encoded, meaning that its size will shirnk compared to byteLength. However we never expect it to grow.
			bufdata.resize(new_sz);
		}
		else
		{
			tz::report("WARNING: GLTF buffer detected without any data uri. Will be zerod.");
		}
			
		this->parsed_buf_count++;
		return
		{
			.label = makeshift_bufname,
			.data = bufdata
		};
	}

	// write the decoded data back into the span. return the new size of the span.
	std::size_t base64_decode(std::span<std::byte> base64_data)
	{
		// 80 is dummy entry.
		std::uint8_t decode_lut[] =
		{
			80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, /* 0 - 15 */
			80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, /* 16 - 31 */
			80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 62, 80, 80, 80, 63, /* 32 - 47 */
			52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 80, 80, 80, 64, 80, 80, /* 48 - 63 */
			80,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, /* 64 - 79 */
			15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 80, 80, 80, 80, 80, /* 80 - 96 */
			80, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, /* 87 - 111 */
			41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 80, 80, 80, 80, 80 /* 112 - 127 */	
		};
		std::size_t encoded_length = base64_data.size();
		tz::assert(encoded_length >= 4 && encoded_length % 4 == 0, "unexpected base64 data length. should be >= 4 and divisible by 4, but it is %zu", encoded_length);
		std::vector<std::uint8_t> decoded_data;
		std::size_t decoded_length = ((encoded_length / 4) * 3);
		decoded_data.reserve(decoded_length);
		// algo: 
		// extract next four bytes
		// get the index value from the lut (reverse lookup)
		// merge the 4 6-bit values into 3 decoded 8-bit values

		// slurp in 4 byte chunks.
		for(std::size_t i = 0; i < base64_data.size(); i += 4)
		{
			// there are special cases with padding chars "=". behaviour is different if those are present.
			std::uint8_t b1 = decode_lut[static_cast<std::size_t>(base64_data[i + 0])];	
			std::uint8_t b2 = decode_lut[static_cast<std::size_t>(base64_data[i + 1])];	
			std::uint8_t b3 = decode_lut[static_cast<std::size_t>(base64_data[i + 2])];	
			std::uint8_t b4 = decode_lut[static_cast<std::size_t>(base64_data[i + 3])];	
			if(base64_data[i + 3] == std::byte{'='})
			{
				if(base64_data[i + 2] == std::byte{'='})
				{
					// two padding chars, only decode the one byte.
					decoded_data.push_back(b1 << 2 | ((b2 & 0xf0) >> 4));
				}
				else
				{
					// one padding char. two bytes to decode.
					decoded_data.push_back(b1 << 2 | ((b2 & 0xf0) >> 4));
					decoded_data.push_back(((b2 & 0x0f) << 4) | (b3 & 0x3c) >> 2);
				}
			}
			else
			{
				// no padding chars. decode all 3 bytes.
				decoded_data.push_back(b1 << 2 | ((b2 & 0xf0) >> 4));
				decoded_data.push_back(((b2 & 0x0f) << 4) | (b3 & 0x3c) >> 2);
				decoded_data.push_back(((b3 & 0x03) << 6) | (b4 & 0x3f));
			}
		}
		tz::assert(decoded_length == decoded_data.size(), "unexpected decoded base64 buffer size. expected %zu, got %zu", decoded_length, decoded_data.size());
		std::memcpy(base64_data.data(), decoded_data.data(), decoded_length);
		return decoded_length;
	}

	std::size_t gltf::parse_data_uri(std::string_view data_uri, std::span<std::byte> byte_view) const
	{
		// according to spec, data uri syntax is as follows:
		// data:[<mediatype>][;base64],<data>
		std::string uri_str{data_uri};
		tz::assert(data_uri.starts_with("data:"), "Invalid buffer uri \"%s\". Topaz only supports embedded data uris.", uri_str.c_str());
		(void)byte_view;
		std::regex syntax_regex{"data:([a-zA-Z\\-\\/\\_]*)?(;base64)?,(.*)"};
		std::smatch syntax_match;
		bool matched = std::regex_match(uri_str, syntax_match, syntax_regex);
		tz::assert(matched, "Invalid data uri format.");
		
		// now get the relevent bits out.
		tz::assert(syntax_match[1].matched, "Could not parse media/mime-type");
		std::string media_type = syntax_match[1].str();
		bool base64 = syntax_match[2].matched;
		tz::assert(base64, "detected non base64 data uri. not supported.");
		tz::assert(syntax_match[3].matched, "did not have any data? malformed data uri");
		std::string data_unparsed = syntax_match[3].str();
		std::size_t data_len = data_unparsed.size();
		if(base64)
		{
			// if base 64, decode the data and return the unchanged size.
			std::span<char> data_span{data_unparsed.data(), data_unparsed.size()};
			data_len = base64_decode(std::as_writable_bytes(data_span));
		}
		std::memcpy(byte_view.data(), data_unparsed.data(), data_len);
		return data_len;
	}

}
