//
// Created by Harry on 23/04/2019.
//

#ifndef TOPAZ_CONSTANTS_HPP
#define TOPAZ_CONSTANTS_HPP
#include "SDL2/SDL.h"
#ifndef GLEW_STATIC
	#define GLEW_STATIC
#endif
#include "GL/glew.h"
#include <algorithm>

namespace tz::consts
{
	namespace numeric
	{
		constexpr float default_smoothness = 8.0f;
		/// 3.14159...
		constexpr float pi = 3.141593;
		/// 2pi
		constexpr float tau = 2.0f * pi;
		/// 2.17...
		constexpr float e = 2.718282;
		///  1 / (2 - cbrt(2))
		constexpr float forest_ruth_coefficient = 1.351207;
		///  1 - (2 * forest_ruth_coefficient)
		constexpr float forest_ruth_complement = 1.0f - (2.0f * forest_ruth_coefficient);
	}

	namespace audio
	{
		constexpr int unused_channel = -1;
	}

	namespace core
	{
		namespace scene::octree
		{
			/// Minimum region of each ScenePartitionNode. Decreasing this increases precision of space partitioning, but at increased overhead.
			constexpr float minimum_node_size_x = 0.1f;
			constexpr float minimum_node_size_y = 0.1f;
			constexpr float minimum_node_size_z = 0.1f;
		}

		namespace window
		{
			/// Window-centre flag. Use this to centre a Window.
			constexpr int centred_window = SDL_WINDOWPOS_CENTERED;
		}
	}

	namespace graphics
	{
		namespace camera
		{
			/// FOV of a Camera if none is specified.
			constexpr float default_fov = tz::consts::numeric::pi / 2.0f;
			/// Near-clip of a Camera if none is specified.
			constexpr float default_near_clip = 0.1f;
			/// Far-clip of a Camera if none is specified.
			constexpr float default_far_clip = 10000.0f;
		}
		namespace mesh::attribute
		{
			constexpr char position_attribute[] = "position";
			constexpr char texcoord_attribute[] = "texcoord";
			constexpr char normal_attribute[] = "normal";
			constexpr char tangent_attribute[] = "tangent";
			constexpr char instance_model_x_attribute[] = "instancing_model_x";
			constexpr char instance_model_y_attribute[] = "instancing_model_y";
			constexpr char instance_model_z_attribute[] = "instancing_model_z";
			constexpr char instance_model_w_attribute[] = "instancing_model_w";
		}
		namespace shader
		{
			namespace sampler
			{
				constexpr std::size_t texture_id = 0;
				constexpr std::size_t cubemap_id = 0;
				constexpr std::size_t normal_map_id = 1;
				constexpr std::size_t parallax_map_id = 2;
				constexpr std::size_t displacement_map_id = 3;
				constexpr std::size_t specular_map_id = 4;
				constexpr std::size_t emissive_map_id = 5;
				// ID from 6-15 inclusive are available for future/custom general-purpose usage.
				/// Meshes using multiple textures should store additional textures in the initial ID and above.
				constexpr std::size_t initial_extra_texture_id = 16;

				constexpr char texture_name[] = "texture_sampler";
				constexpr char normal_map_name[] = "normal_map_sampler";
				constexpr char specular_map_name[] = "specular_map_sampler";
				constexpr char emissive_map_name[] = "emissive_map_sampler";
				constexpr char extra_texture_sampler_prefix[] = "extra_texture_sampler";
			}

			constexpr char render_shader_model_uniform_name[] = "m";
			constexpr char render_shader_view_uniform_name[] = "v";
			constexpr char render_shader_projection_uniform_name[] = "p";
			/**
			* Vertex, Tessellation Control, Tessellation Evaluation, Geometry, Fragment.
			*/
			constexpr std::size_t maximum_shaders = 5;
			constexpr std::size_t maximum_uniforms = GL_MAX_UNIFORM_LOCATIONS;
		}

		namespace framebuffer
		{
			/**
			* Minimum of implementation and 32. This is because if hardware allows 64 attachments, OpenGL headers currently don't even specify 32+ attachments (it goes to GL_DEPTH_ATTACHMENT). For this reason, it is the minimum of the two, for a fair compromise.
			*/
			constexpr int maximum_attachments = std::min(GL_MAX_COLOR_ATTACHMENTS, 32);
		}

		namespace texture
		{
			constexpr float default_parallax_map_scale = 0.04f;
			constexpr float default_parallax_map_offset = -0.5f;
			constexpr float default_displacement_factor = 0.25f;
		}
	}
}

#endif //TOPAZ_CONSTANTS_HPP