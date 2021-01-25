#include "core/core.hpp"
#include "core/debug/print.hpp"
#include "core/matrix_transform.hpp"
#include "gl/tz_stb_image/image_reader.hpp"
#include "gl/shader.hpp"
#include "gl/shader_compiler.hpp"
#include "gl/manager.hpp"
#include "gl/mesh_loader.hpp"
#include "gl/buffer.hpp"
#include "gl/frame.hpp"
#include "gl/modules/ssbo.hpp"
#include "gl/modules/ubo.hpp"
#include "gl/texture.hpp"
#include "render/device.hpp"
#include "GLFW/glfw3.h"
#include "dui/imgui_context.hpp"
#include "gl/resource_writer.hpp"
#include "gl/modules/bindless_sampler.hpp"
#include "render/asset.hpp"
#include "render/scene.hpp"
#include "render/pipeline.hpp"

const char *vtx_shader_src = R"glsl(
	#version 460
	layout (location = 0) in vec3 aPos;
	layout (location = 1) in vec2 aTexcoord;
	#ssbo matrices
	{
		mat4 mvp[512];
	};
	out vec2 texcoord;
	void main()
	{
		gl_Position = mvp[gl_DrawID] * vec4(aPos.x, aPos.y, aPos.z, 1.0);
		texcoord = aTexcoord;
	}
	)glsl";
const char *frg_shader_src = R"glsl(
	#version 460
	#extension GL_ARB_bindless_texture : require
	layout(location = 0) out vec4 FragColor;
	in vec2 texcoord;
	#ssbo texture_block
	{
		tz_bindless_sampler textures[8];
	};
	void main()
	{
		FragColor = texture(textures[1], texcoord);
	}
	)glsl";
const char *postprocess_vtx_shader_src = R"glsl(
	#version 460
	layout (location = 0) in vec3 aPos;
	layout (location = 1) in vec2 aTexcoord;
    out vec2 texcoord;
	void main()
	{
		gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
        texcoord = aTexcoord;
	}
	)glsl";
const char *postprocess_frg_shader_src = R"glsl(
	#version 460
	out vec4 FragColor;
	in vec2 texcoord;
	uniform sampler2D rto_sampler;

	#ubo postprocess_strategy
	{
		int strategy_id;
	};

	vec4 invert(vec4 col)
	{
		return vec4(1.0 - col.r, 1.0 - col.g, 1.0 - col.b, 1.0);
	}

	vec4 blur(vec4 col, bool horizontal)
	{
		const int kernel_size = 5;
		float weight[kernel_size] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

		vec2 tex_offset = 1.0 / textureSize(rto_sampler, 0); // gets size of single texel
		vec3 result = col.rgb * weight[0];
		if(horizontal)
		{
			for(int i = 0; i < kernel_size; i++)
			{
				result += texture(rto_sampler, texcoord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
				result += texture(rto_sampler, texcoord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
			}
		}
		else
		{
			for(int i = 0; i < kernel_size; i++)
			{
			result += texture(rto_sampler, texcoord + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
			result += texture(rto_sampler, texcoord - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
			}
		}
		// then perform tone mapping...

		return vec4(result, 1.0);
	}

	void main()
	{
		vec4 tex = texture(rto_sampler, texcoord);
		if(strategy_id == 0)
		{
			// No change.
			FragColor = tex;
		}
		else if(strategy_id == 1)
		{
			// Invert colour.
			FragColor = invert(tex);
		}
		else if(strategy_id == 2)
		{
			// Blur colour
			FragColor = blur(blur(tex, true), false);
		}
	}
	)glsl";

class PostprocessDemoWindow : public tz::ext::imgui::ImGuiWindow
{
public:
	PostprocessDemoWindow(int& strategy_id): tz::ext::imgui::ImGuiWindow("Topaz Postprocess Demo"), strategy_id(strategy_id){}
	virtual void render() override
	{
		ImGui::Begin("Topaz Postprocess Demo", &this->visible);
		ImGui::RadioButton("No post-processing", &this->strategy_id, 0);
		ImGui::RadioButton("Invert colours", &this->strategy_id, 1);
		ImGui::RadioButton("Gaussian blur", &this->strategy_id, 2);
		ImGui::End();
	}
private:
	int& strategy_id;
};

int main()
{
	constexpr std::size_t max_elements = 512;
	constexpr std::size_t max_textures = 8;
	// Minimalist Graphics Demo.
	tz::core::initialise("Topaz Postprocess Demo");
	{
		tz::gl::Manager m;
		tz::gl::Object& o = *m;

		// Track it in imgui.
		tz::ext::imgui::track_object(&o);

		tz::gl::p::SSBOModule* ssbo_module = nullptr;
		tz::gl::p::UBOModule* ubo_module = nullptr;
		std::string vtx_result;
		std::string frg_result;
		tz::gl::ShaderPreprocessor pre{vtx_shader_src};
		{
			std::size_t ssbo_module_id = pre.emplace_module<tz::gl::p::SSBOModule>(&o);
			std::size_t ubo_module_id = pre.emplace_module<tz::gl::p::UBOModule>(&o);
			pre.emplace_module<tz::gl::p::BindlessSamplerModule>();
			pre.preprocess();
			vtx_result = pre.result();
			pre.set_source(frg_shader_src);
			pre.preprocess();
			frg_result = pre.result();
			ssbo_module = static_cast<tz::gl::p::SSBOModule*>(pre[ssbo_module_id]);
			ubo_module = static_cast<tz::gl::p::UBOModule*>(pre[ubo_module_id]);
		}
		topaz_assert(ssbo_module->size() == 2, "SSBO Module had unexpected number of buffers. Expected 2, got ", ssbo_module->size());
		topaz_assert(ubo_module->size() == 0, "UBO Module had unexpected number of buffers. Expected 0, got ", ssbo_module->size());

		std::size_t ssbo_id = ssbo_module->get_buffer_id(0);
		std::size_t tex_ssbo_id = ssbo_module->get_buffer_id(1);

		tz::gl::SSBO* ssbo = o.get<tz::gl::BufferType::ShaderStorage>(ssbo_id);
		tz::gl::SSBO* tex_ssbo = o.get<tz::gl::BufferType::ShaderStorage>(tex_ssbo_id);

		tz::gl::IndexedMesh square;
		square.vertices.push_back(tz::gl::Vertex{{{-1.0f, -1.0f, 0.0f}}, {{0.0f, 0.0f}}, {{}}, {{}}, {{}}});
		square.vertices.push_back(tz::gl::Vertex{{{1.0f, -1.0f, 0.0f}}, {{1.0f, 0.0f}}, {{}}, {{}}, {{}}});
		square.vertices.push_back(tz::gl::Vertex{{{1.0f, 1.0f, 0.0f}}, {{1.0f, 1.0f}}, {{}}, {{}}, {{}}});
		
		square.vertices.push_back(tz::gl::Vertex{{{-1.0f, -1.0f, 0.0f}}, {{0.0f, 0.0f}}, {{}}, {{}}, {{}}});
		square.vertices.push_back(tz::gl::Vertex{{{1.0f, 1.0f, 0.0f}}, {{1.0f, 1.0f}}, {{}}, {{}}, {{}}});
		square.vertices.push_back(tz::gl::Vertex{{{-1.0f, 1.0f, 0.0f}}, {{0.0f, 1.0f}}, {{}}, {{}}, {{}}});
		square.indices = {0, 1, 2, 3, 4, 5};

		tz::gl::IndexedMesh monkey_head = tz::gl::load_mesh("res/models/monkeyhead.obj");
		const tz::Vec3 cam_pos{{0.0f, 0.0f, 5.0f}};
		tz::debug_printf("monkey head data size = %zu bytes, indices size = %zu bytes", monkey_head.data_size_bytes(), monkey_head.indices_size_bytes());

		tz::gl::Image<tz::gl::PixelRGB8> rgba_checkerboard = tz::ext::stb::read_image<tz::gl::PixelRGB8>("res/textures/bricks.jpg");
		tz::gl::Texture checkerboard;
		checkerboard.set_parameters(tz::gl::default_texture_params);
		checkerboard.set_data(rgba_checkerboard);
		checkerboard.make_terminal();
		
		tz::gl::Image<tz::gl::PixelRGB8> metal_img = tz::ext::stb::read_image<tz::gl::PixelRGB8>("res/textures/metal.jpg");
		tz::gl::Texture metal;
		metal.set_parameters(tz::gl::default_texture_params);
		metal.set_data(metal_img);
		metal.make_terminal();

		tz::Vec3 triangle_pos{{-25.0f, -25.0f, -25.0f}};
		float rotation_factor = 0.02f;
		float x_factor = 2.5f;
		float y_factor = 2.5f;

		tz::gl::Manager::Handle square_handle = m.add_mesh(square);
		tz::gl::Manager::Handle monkeyhead_handle = m.add_mesh(monkey_head);

		// UBO stores mesh transform data (mvp)
		ssbo->terminal_resize(sizeof(tz::Mat4) * max_elements);
		// Scene uses UBO resource data.
		tz::render::Scene<tz::render::SceneElement> scene{ssbo->map()};

		float rotation_y = 0.0f;

		tz::core::IWindow& wnd = tz::core::get().window();
		wnd.register_this();

		wnd.get_frame()->set_clear_color(0.3f, 0.15f, 0.0f);
		// Scene setup.
		// Meshes
		tz::render::AssetBuffer::Index monkey_mesh_idx = scene.add_mesh({&m, monkeyhead_handle});
		// Textures
		tex_ssbo->resize(sizeof(tz::gl::BindlessTextureHandle) * max_textures);
		{
			tz::mem::UniformPool<tz::gl::BindlessTextureHandle> tex_pool = tex_ssbo->map_uniform<tz::gl::BindlessTextureHandle>();
			tz::gl::BindlessTextureHandle checkerboard_handle = checkerboard.get_terminal_handle();
			tz::gl::BindlessTextureHandle metal_handle = metal.get_terminal_handle();
			tex_pool.set(0, metal_handle);
			tex_pool.set(1, checkerboard_handle);
			tex_ssbo->unmap();
		}

		tz::gl::ShaderCompiler cpl;
		tz::gl::ShaderProgram prg;
		tz::gl::Shader* vs = prg.emplace(tz::gl::ShaderType::Vertex);
		vs->upload_source(vtx_result);
		tz::gl::Shader* fs = prg.emplace(tz::gl::ShaderType::Fragment);
		fs->upload_source(frg_result);

		auto cpl_diag = cpl.compile(*vs);
		topaz_assert(cpl_diag.successful(), "Shader Vtx Compilation Fail: ", cpl_diag.get_info_log());
		cpl_diag = cpl.compile(*fs);
		topaz_assert(cpl_diag.successful(), "Shader Frg Compilation Fail: ", cpl_diag.get_info_log());

		auto lnk_diag = cpl.link(prg);
		topaz_assert(lnk_diag.successful(), "Shader Linkage Fail: ", lnk_diag.get_info_log());

        tz::gl::ShaderProgram postprocess_shader;
        tz::gl::Shader* postpro_vs = postprocess_shader.emplace(tz::gl::ShaderType::Vertex);
        postpro_vs->upload_source(postprocess_vtx_shader_src);
        tz::gl::Shader* postpro_fs = postprocess_shader.emplace(tz::gl::ShaderType::Fragment);
		pre.set_source(postprocess_frg_shader_src);
		pre.preprocess();
		topaz_assert(ubo_module->size() == 1, "UBO Module had unexpected number of buffers. Expected 1, got ", ssbo_module->size());
        postpro_fs->upload_source(pre.result());

		std::size_t ubo_id = ubo_module->get_buffer_id(0);
		tz::gl::UBO* pass2_tex_ubo = o.get<tz::gl::BufferType::UniformStorage>(ubo_id);
		pass2_tex_ubo->terminal_resize(sizeof(int));
		int* strategy_id = nullptr;
		{
			tz::mem::Block strategy_id_block = pass2_tex_ubo->map();
			strategy_id = new (strategy_id_block.begin) int{0};
		}

		tz::ext::imgui::emplace_window<PostprocessDemoWindow>(*strategy_id);

        cpl_diag = cpl.compile(*postpro_vs);
		topaz_assert(cpl_diag.successful(), "Shader Vtx Compilation Fail: ", cpl_diag.get_info_log());
		cpl_diag = cpl.compile(*postpro_fs);
		topaz_assert(cpl_diag.successful(), "Shader Frg Compilation Fail: ", cpl_diag.get_info_log());

		lnk_diag = cpl.link(postprocess_shader);
		topaz_assert(lnk_diag.successful(), "Shader Linkage Fail: ", lnk_diag.get_info_log());

		for(std::size_t i = 0; i < max_elements; i++)
		{
			tz::render::SceneElement ele{monkey_mesh_idx};
			ele.transform.position = tz::Vec3{1.0f, 1.0f, 1.0f};
			ele.transform.rotation = tz::Vec3{0.0f, 0.0f, 0.0f};
			ele.camera.fov = 1.57f;
			ele.camera.aspect_ratio = 1920.0f/1080.0f;
			ele.camera.near = 0.1f;
			ele.camera.far = 1000.0f;
			scene.add(ele);
		}

        // Pipeline is as follows:
        /*
        * 1: Render scene into a texture via FBO.
        * 2: Render quad using the above texture bound, using the inverse colour shader, into the window's frame.
        */

        tz::render::Pipeline pl;
        pl.add({nullptr, &prg, &o});
        auto dev = [&pl]()->tz::render::Device&{return *pl[0];};
        dev().add_resource_buffer(ssbo);
		dev().add_resource_buffer(tex_ssbo);
		dev().set_handle(m.get_indices());
        
        // Make fbo with same size as window.
        tz::gl::Frame fbo{static_cast<unsigned int>(wnd.get_width()), static_cast<unsigned int>(wnd.get_height())};
        tz::gl::Texture& render_to_texture = fbo.emplace_texture(tz::gl::FrameAttachment::Colour);
        tz::gl::TextureDataDescriptor desc;
        desc.component_type = tz::gl::TextureComponentType::Byte;
        desc.internal_format = tz::gl::TextureInternalFormat::RedGreenBlueAlpha8;
        desc.format = tz::gl::TextureFormat::RedGreenBlueAlpha;
        desc.width = wnd.get_width();
        desc.height = wnd.get_height();
        render_to_texture.resize(desc);
        render_to_texture.set_parameters(tz::gl::default_texture_params);
		tz::gl::TextureDataDescriptor desc2 = desc;
		desc2.internal_format = tz::gl::TextureInternalFormat::DepthUnsized;
        fbo.emplace_renderbuffer(tz::gl::FrameAttachment::Depth, desc2);
		fbo.bind();
        fbo.set_output_attachment(tz::gl::FrameAttachment::Colour);
        pl.add({wnd.get_frame(), &postprocess_shader, &o});
        tz::render::Device& dev2 = *pl[1];
        topaz_assert(fbo.complete(), "FBO Incomplete");
        dev().set_frame(&fbo);
        postprocess_shader.attach_texture(0, &render_to_texture, "rto_sampler");
        dev2.set_handle(m.get_indices());
		dev2.add_resource_buffer(pass2_tex_ubo);

		tz::gl::IndexSnippetList quad_list;
        quad_list.emplace_range(m, square_handle);
        dev2.set_indices(quad_list);
        
		while(!wnd.is_close_requested())
		{
			rotation_y += rotation_factor;

			pl.clear();
			for(std::size_t i = 0; i < max_elements; i++)
			{
				tz::Vec3 cur_pos = triangle_pos;
				// Three meshes in a horizontal line.
				cur_pos[0] += (i % static_cast<std::size_t>(std::sqrt(max_elements))) * x_factor;
				cur_pos[1] += std::floor(i / std::sqrt(max_elements)) * y_factor;

				tz::render::SceneElement& cur_ele = scene.get(i);
				cur_ele.transform.position = cur_pos;
				cur_ele.transform.rotation[1] = rotation_y;
				cur_ele.camera.position = cam_pos;
			}

			pl.clear();
			
			scene.configure(dev());
			pl.render();
			tz::core::update();
			wnd.update();
		}
	}
	tz::core::terminate();
}