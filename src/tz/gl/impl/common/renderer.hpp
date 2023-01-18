#ifndef TOPAZ_GL2_IMPL_FRONTEND_COMMON_RENDERER_HPP
#define TOPAZ_GL2_IMPL_FRONTEND_COMMON_RENDERER_HPP
#include "tz/core/memory.hpp"
#include "tz/gl/api/renderer.hpp"
#include "tz/gl/impl/common/shader.hpp"
#include "imgui.h"
#undef assert

namespace tz::gl
{
	namespace detail
	{
		constexpr std::array<const char*, static_cast<int>(tz::gl::RendererOption::Count)> renderer_option_strings =
		{
			"No Depth Testing",
			"Alpha Blending",
			"Render Wait",
			"No Clear Output",
			"No Present",
			"Final Debug UI Renderer (Internal)",
			"Internal",
		};
	}
	/**
	 * @ingroup tz_gl2_renderer
	 * Helper class which can be used to generate a @ref RendererEditRequest.
	 */
	class RendererEditBuilder
	{
	public:
		RendererEditBuilder() = default;
		/**
		 * Make amendments to the compute configuration of the renderer. See @ref RendererComputeEditRequest for details.
		 */
		RendererEditBuilder& compute(RendererEdit::ComputeConfig req);
		/**
		 * Make amendments to the current render state. See @ref RenderStateEditRequest for details.
		 */
		RendererEditBuilder& render_state(RendererEdit::RenderConfig req);
		/**
		 * Make amendments to an existing image resource. See @ref RendererImageComponentResizeRequest for details.
		 */
		RendererEditBuilder& image_resize(RendererEdit::ImageResize req);
		/**
		 * Make amendments to an existing buffer resource. See @ref RendererBufferComponentResizeRequest for details.
		 */
		RendererEditBuilder& buffer_resize(RendererEdit::BufferResize req);
		RendererEditBuilder& write(RendererEdit::ResourceWrite req);
		/**
		 * Retrieve a @ref RendererEditRequest corresponding to all edits specified within the builder so far.
		 */
		RendererEditRequest build() const;
	private:
		RendererEditRequest request = {};
	};
	/**
	 * @ingroup tz_gl2_renderer
	 * Helper struct which the user can use to specify which inputs, resources they want and where they want a Renderer to render to. This is likely going to be refactored at some point because it doesn't do any Vulkan-specific magic.
	 */
	class renderer_infoCommon
	{
	public:
		renderer_infoCommon();
		// Satisfies renderer_info_type.
		/**
		 * Retrieve the number of resources.
		 */
		unsigned int resource_count() const;
		/**
		 * Retrieve the resource corresponding to the given handle.
		 * @param Handle handle returned from a previous call to `add_resource`. If this handle came from a different renderer_info, the behaviour is undefined.
		 * @return Pointer to the resource.
		 */
		const IResource* get_resource(ResourceHandle handle);
		/**
		 * Retrieve a span containing all of the specified resources. Size of the span is guaranteed to be equal to @ref resource_count()
		 */
		std::vector<const IResource*> get_resources() const;
		std::span<const renderer_handle> get_dependencies() const;
		std::span<const icomponent* const> get_components() const;
		/**
		 * Add a new resource, which will be used by a Renderer which is created from this helper struct.
		 *
		 * @param resource Resource which will be owned by a renderer.
		 * @return Handle corresponding to the resource. If you want to retrieve the resource later, you should keep ahold of this handle.
		 */
		ResourceHandle add_resource(const IResource& resource);
		ResourceHandle ref_resource(icomponent* component);
		ResourceHandle ref_resource(renderer_handle ren, ResourceHandle res);
		/**
		 * Renderers always render into something. By default, it renders to the window (only one window is supported so no confusion there). You can however set it to render into something else, such as a @ref TextureOutput if you want to render into the resource of another Renderer.
		 */
		void set_output(const IOutput& output);
		/**
		 * Retrieve the current render output (read-only). This may return nullptr, meaning that the main window will be rendered into.
		 */
		const IOutput* get_output() const;
		/**
		 * Retrieve the currently specified options which will be used by the renderer.
		 */
		const RendererOptions& get_options() const;
		/**
		 * Set the currently specified options which will be used by the renderer.
		 */
		void set_options(RendererOptions options);
		/**
		 * Set the pending renderer to be dependent on the specified renderer.
		 *
		 * This means that when render() is invoked, the GPU will wait on completion of render-work of the specified renderer before the render work of this renderer begins. This also means that the specified renderer *must* run each time this is ran, ahead-of-time.
		 */
		void add_dependency(renderer_handle dependency);
		/**
		 * Read/write information about the state of the renderer when it is created.
		 */
		RenderState& state();
		/**
		 * Read-only information about the state of the renderer when it is created.
		 */
		const RenderState& state() const;
		/**
		 * Read/write information about the shader that will be built for the renderer.
		 */
		ShaderInfo& shader();
		/**
		 * Read-only information about the shader that will be built for the renderer.
		 */
		const ShaderInfo& shader() const;
		/**
		 * Set the debug name of the spawned renderer. By default, the debug name is a compact description of the renderer.
		 * @note This only has an affect on debug builds.
		 */
		void debug_name(std::string debug_name);
		/**
		 * Retrieve the debug name which will be used for the spawned renderer.
		 */
		std::string debug_get_name() const;
	private:
		std::size_t real_resource_count() const;
		/// Stores all provided resources. It is assumed that their lifetime is valid for the entirety of this helper struct's lifetime.
		std::vector<std::unique_ptr<IResource>> resources = {};
		/// Stores all provided components. In this context, components act as references to existing resources owned by another renderer.
		std::vector<icomponent*> components = {};
		/// Output. Can be null, which defaults to rendering into the main window.
		std::unique_ptr<IOutput> output = nullptr;
		/// Specifies which extra features the Renderer will have.
		RendererOptions options = {};
		/// Describes render state. It could change.
		RenderState renderer_state = {};
		/// List of renderers we are dependent on.
		std::vector<renderer_handle> dependencies = {};
		/// Describes the shader sources used.
		ShaderInfo shader_info;
		/// The clear value for colour attachments.
		hdk::vec4 clear_colour = {0.0f, 0.0f, 0.0f, 1.0f};
		/// Workgroup sizes if we're doing compute.
		hdk::vec3ui compute_kernel = {1u, 1u, 1u};
		std::string dbg_name = "";
	};
	static_assert(renderer_info_type<renderer_infoCommon>);

	template<class Asset>
	class AssetStorageCommon
	{
	public:
		using AssetHandle = hdk::handle<Asset>;
		AssetStorageCommon(std::span<const Asset* const> assets = {}):
		asset_storage()
		{
			for(const Asset* asset : assets)
			{
				if(asset == nullptr)
				{
					this->asset_storage.push_back(nullptr);
				}
				else
				{
					this->asset_storage.push_back(asset->unique_clone());
				}
			}
		}

		unsigned int count() const
		{
			return this->asset_storage.size();
		}
		
		const Asset* get(AssetHandle handle) const
		{
			if(handle == hdk::nullhand) return nullptr;
			std::size_t handle_val = static_cast<std::size_t>(static_cast<hdk::hanval>(handle));
			return this->asset_storage[handle_val].get();
		}

		Asset* get(AssetHandle handle)
		{
			if(handle == hdk::nullhand) return nullptr;
			std::size_t handle_val = static_cast<std::size_t>(static_cast<hdk::hanval>(handle));
			return this->asset_storage[handle_val].get();
		}

		void set(AssetHandle handle, Asset* value)
		{
			std::size_t handle_val = static_cast<std::size_t>(static_cast<hdk::hanval>(handle));
			hdk::assert(!this->asset_storage[handle_val].owning(), "AssetStorageCommon: Try to set specific asset value, but the asset at that handle is not a reference (it is owned by us)");
			this->asset_storage[handle_val] = value;
		}
	private:
		std::vector<maybe_owned_ptr<Asset>> asset_storage;
	};

	/**
	 * @ingroup tz_gl2_renderer
	 * Helper function which displays render-api-agnostic information about renderers.
	 */
	void common_renderer_dbgui(RendererType auto& renderer)
	{
		if(renderer.is_null())
		{
			ImGui::Text("Null Renderer");
			return;
		}
		ImGui::PushID(&renderer);
		ImGui::Text("Renderer Name:");
		ImGui::SameLine();
		ImGui::TextColored(ImVec4{1.0f, 0.6f, 0.6f, 1.0f}, "%s", renderer.debug_get_name().data());
		if(renderer.resource_count() > 0)
		{
			if(ImGui::CollapsingHeader("Resources"))
			{
				unsigned int rcount = renderer.resource_count() - 1;
				ImGui::Text("Resource Count: %u", renderer.resource_count());
				static int res_id = 0;
				res_id = std::clamp(res_id, 0, static_cast<int>(rcount));
				ImGui::SliderInt("Resource ID:", &res_id, 0, rcount);

				// Display information about current resource.
				ImGui::Indent();
				renderer.get_resource(static_cast<hdk::hanval>(res_id))->dbgui();
				ImGui::Unindent();
			}
		}
		else
		{
			ImGui::Text("Renderer has no resources.");
		}
		if(ImGui::CollapsingHeader("Renderer State"))
		{
			ImGui::TextColored(ImVec4{1.0f, 0.6f, 0.6f, 1.0f}, "Graphics");
			ImGui::Indent();
			// Graphics - Index Buffer
			{
				auto han = renderer.get_state().graphics.index_buffer;
				auto* comp = renderer.get_component(han);
				if(comp != nullptr && ImGui::CollapsingHeader("Index Buffer"))
				{
					ImGui::Text("[Resource %zu]", static_cast<std::size_t>(static_cast<hdk::hanval>(han)));
					ImGui::Indent();
					comp->get_resource()->dbgui();
					ImGui::Unindent();
				}
			}
			// Graphics - Draw Buffer
			{
				auto han = renderer.get_state().graphics.draw_buffer;
				auto* comp = renderer.get_component(han);
				if(comp != nullptr && ImGui::CollapsingHeader("Draw Indirect Buffer"))
				{
					ImGui::Text("[Resource %zu]", static_cast<std::size_t>(static_cast<hdk::hanval>(han)));
					ImGui::Indent();
					comp->get_resource()->dbgui();
					ImGui::Unindent();
				}
			}
			ImGui::Spacing();
			// Graphics - Clear Colour
			{
				auto col = renderer.get_state().graphics.clear_colour;
				if(ImGui::DragFloat4("Clear Colour", col.data().data(), 0.02f, 0.0f, 1.0f))
				{
					ImGui::Text("Direct edit is not yet supported.");
				}
			}
			// Graphics - Tri Count
			{
				if(renderer.get_state().graphics.draw_buffer != hdk::nullhand)
				{
					ImGui::Text("Triangle Count: Indirect Buffer");
				}
				else
				{
					ImGui::Text("Triangle Count: %zu", renderer.get_state().graphics.tri_count);
				}
			}
			ImGui::Unindent();
			ImGui::TextColored(ImVec4{1.0f, 0.6f, 0.6f, 1.0f}, "Compute");
			ImGui::Indent();
			// Compute - Kernel
			{
				auto kern = static_cast<hdk::vec3i>(renderer.get_state().compute.kernel);
				if(ImGui::DragInt3("Kernel", kern.data().data(), 0.25f, 0, 64))
				{
					ImGui::Text("Direct edit is not yet supported.");
				}
			}
			ImGui::Unindent();
		}
		if(!renderer.get_options().empty() && ImGui::CollapsingHeader("Renderer Options"))
		{
			for(RendererOption option : renderer.get_options())
			{
				ImGui::Text("%s", detail::renderer_option_strings[static_cast<int>(option)]);
			}
			#if HDK_DEBUG
				ImGui::PushTextWrapPos();
				ImGui::TextDisabled("Note: In debug builds, extra options might be present that you did not ask for. These are added to allow the debug-ui to display ontop of your rendered output.");
				ImGui::PopTextWrapPos();
			#endif //TZ_DEBUG
		}
		if(ImGui::CollapsingHeader("On-Demand Edits"))
		{
			ImGui::Text("Wireframe Mode");
			if(ImGui::Button("Enable"))
			{
				renderer.edit
				(
					tz::gl::RendererEditBuilder{}
					.render_state({.wireframe_mode = true})
					.build()
				);
			}
			ImGui::SameLine();
			if(ImGui::Button("Disable"))
			{
				renderer.edit
				(
					tz::gl::RendererEditBuilder{}
					.render_state({.wireframe_mode = false})
					.build()
				);
			}
			ImGui::Spacing();
		}
		ImGui::Separator();
		ImGui::PopID();
	}

}

#endif // TOPAZ_GL2_IMPL_FRONTEND_COMMON_RENDERER_HPP
