#include "tz/tz.hpp"
#include "tz/core/job/job.hpp"
#include "tz/core/profile.hpp"
#include "tz/core/time.hpp"
#include "tz/dbgui/dbgui.hpp"
#include "tz/core/matrix_transform.hpp"

#include "tz/ren/animation.hpp"

//tmp
#include "tz/io/ttf.hpp"
struct dbgui_data_t
{
	bool mesh_renderer_enabled = false;
} dbgui_data;
void dbgui_init();

int main()
{
	tz::initialise
	({
		.name = "tz_animation_demo"
	});
	{
		tz::io::ttf font = tz::io::ttf::from_file("../../demo/gl/tz_animation_demo/res/simplestory.ttf");
		// wanna sanity check ur TRS bro?
		//tz::trs trsa{.translate = {0.5f, 150.0f, 0.3f}, .rotate = {0.0f, 0.0f, 0.0f, 1.0f}, .scale = {1.0f, 1.0f, 1.0f}};
		//tz::mat4 mata = trsa.matrix();
		//tz::trs trsb{.translate = {-0.5f, 0.0f, 0.0f}, .rotate = tz::quat{0.5f, 0.0f, 0.0f, 0.5f}.normalised(), .scale = {0.5f, 0.5f, 0.5f}};
		//tz::mat4 matb = trsb.matrix();

		//tz::vec4 poshom{0.5f, -10.0f, 0.0f, 1.0f};
		//tz::mat4 matprod = mata * matb;
		//tz::trs trsprod = trsb.combined(trsa);
		//tz::mat4 trsprodmat = trsprod.matrix();
		//for(std::size_t i = 0; i < 4; i++)
		//{
			//for(std::size_t j = 0; j < 4; j++)
			//{
				//tz::assert(trsprodmat(i, j) - matprod(i, j) < 0.1f);
			//}
		//}

		dbgui_init();

		tz::ren::animation_renderer ar;
		/*
		auto pkg = ar.add_gltf(tz::io::gltf::from_file("../../demo/gl/tz_animation_demo/res/sponza.glb"));
		tz::ren::animation_renderer::object_handle right_hand = pkg.objects[108];
		auto sword_parent = ar.add_object
		({
			.trs = {{0.074f, 0.011f, 0.536f}, {0.734f, 0.0f, 0.0f, 0.679f}, {0.298f, 0.372f, 0.347f}},
			.parent = right_hand
		});
		ar.add_gltf(tz::io::gltf::from_file("../../demo/gl/tz_animation_demo/res/animated_sword.glb"), sword_parent);
		*/
		auto pkg = ar.add_gltf(tz::io::gltf::from_file("../../demo/gl/tz_animation_demo/res/human_male.glb"));
		tz::ren::animation_renderer::override_package opkg
		{
			.pkg = pkg,
			.overrides = {tz::ren::animation_renderer::override_flag::mesh, tz::ren::animation_renderer::override_flag::texture},
		};
		auto pkg2 = ar.add_gltf(tz::io::gltf::from_file("../../demo/gl/tz_animation_demo/res/human_male.glb"), opkg);
		auto sword_parent = ar.add_object
		({
			.trs = {{36.702f, 24.468f, -6.842f}, {0.554f, 0.818f, -0.154f, -0.0f}, {25.0f, 25.0f, 25.0f}},
			.parent = pkg.objects[38]
		});
		//ar.add_gltf(tz::io::gltf::from_file("../../demo/gl/tz_animation_demo/res/animated_sword.glb"), sword_parent);
		ar.add_gltf(tz::io::gltf::from_file("../../demo/gl/tz_animation_demo/res/ashbringer.glb"), sword_parent);
		ar.append_to_render_graph();

		auto empties = ar.find_objects_by_name("Unnamed Object");
		tz::report("%zu empties", empties.size());

		tz::duration update_timer = tz::system_time();
		while(!tz::window().is_close_requested())
		{
			TZ_FRAME_BEGIN;
			tz::begin_frame();
			// draw
			tz::gl::get_device().render();
			ar.update((tz::system_time() - update_timer).seconds<float>());
			update_timer = tz::system_time();
			// advance dbgui
			tz::dbgui::run([&ar]()
			{
				if(dbgui_data.mesh_renderer_enabled)
				{
					if(ImGui::Begin("Animation Renderer", &dbgui_data.mesh_renderer_enabled))
					{
						ar.dbgui();
						ImGui::End();
					}
				}
			});
			tz::end_frame();
			TZ_FRAME_END;
		}
	}
	tz::terminate();
}

// invoked when dbgui needs to set up initial state.
void dbgui_init()
{
	tz::dbgui::game_menu().add_callback([]()
	{
		ImGui::MenuItem("Animation Renderer", nullptr, &dbgui_data.mesh_renderer_enabled);
	});
}