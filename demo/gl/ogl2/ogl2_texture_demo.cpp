#include "tz/core/tz.hpp"
#include "tz/gl/imported_shaders.hpp"
#include "tz/gl/impl/backend/ogl2/tz_opengl.hpp"
#include "tz/gl/impl/backend/ogl2/framebuffer.hpp"
#include "tz/gl/impl/backend/ogl2/shader.hpp"
#include "tz/gl/impl/backend/ogl2/image.hpp"
#include "tz/gl/impl/backend/ogl2/buffer.hpp"
#include "tz/gl/impl/backend/ogl2/vertex_array.hpp"

#include ImportedShaderHeader(texture_demo, vertex)
#include ImportedShaderHeader(texture_demo, fragment)

int main()
{
	tz::initialise
	({
		.name = "ogl2_texture_demo",
	});
	{
		using namespace tz::literals;
		using namespace tz::gl::ogl2;
		VertexArray vao;

		OGLString vtx_src = std::string(ImportedShaderSource(texture_demo, vertex));
		OGLString frg_src = std::string(ImportedShaderSource(texture_demo, fragment));

		Shader shader
		{{
			.modules =
			{
				{
					.type = ShaderType::Vertex,
					.code = vtx_src
				},
				{
					.type = ShaderType::Fragment,
					.code = frg_src
				}
			 }
		}};
		Image image
		{{
			.format = ImageFormat::RGBA32,
			.dimensions = {2u, 2u},
			.sampler =
			{
				.min_filter = LookupFilter::Nearest,
				.mag_filter = LookupFilter::Nearest,
				.address_mode_s = AddressMode::ClampToEdge,
				.address_mode_t = AddressMode::ClampToEdge,
				.address_mode_r = AddressMode::ClampToEdge
			}
		}};
		std::vector<unsigned char> imgdata
		{
			0b1111'1111_uc,
			0b0000'0000_uc,
			0b0000'0000_uc,
			0b1111'1111_uc,

			0b0000'0000_uc,
			0b1111'1111_uc,
			0b0000'0000_uc,
			0b1111'1111_uc,

			0b0000'0000_uc,
			0b1111'1111_uc,
			0b0000'0000_uc,
			0b1111'1111_uc,

			0b1111'1111_uc,
			0b0000'0000_uc,
			0b0000'0000_uc,
			0b1111'1111_uc

		};
		image.set_data(std::as_bytes(std::span<unsigned char>(imgdata)));
		Buffer image_handle_storage_buffer
		{{
			.target = BufferTarget::Uniform,
			.residency = BufferResidency::Dynamic,
			.size_bytes = sizeof(Image::BindlessTextureHandle) * 1
		}};
		image.make_bindless();
		{
			auto handle_span = image_handle_storage_buffer.map_as<Image::BindlessTextureHandle>();
			handle_span.front() = image.get_bindless_handle();
		}
		Framebuffer frame = Framebuffer::null();
		while(!tz::window().is_close_requested())
		{
			tz::window().update();
			frame.bind();
			frame.clear();
			shader.use();
			image_handle_storage_buffer.bind_to_resource_id(0);
			vao.draw(1);
		}
	}
	tz::terminate();
}
