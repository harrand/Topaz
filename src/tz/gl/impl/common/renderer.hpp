#ifndef TOPAZ_GL2_IMPL_FRONTEND_COMMON_RENDERER_HPP
#define TOPAZ_GL2_IMPL_FRONTEND_COMMON_RENDERER_HPP
#include "tz/gl/api/renderer.hpp"
#include "tz/gl/api/shader.hpp"
#include "tz/core/memory/maybe_owned_ptr.hpp"
#include "imgui.h"
#undef assert

namespace tz::gl
{
	template<class Asset>
	class AssetStorageCommon
	{
	public:
		using AssetHandle = tz::handle<Asset>;
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
			if(handle == tz::nullhand) return nullptr;
			std::size_t handle_val = static_cast<std::size_t>(static_cast<tz::hanval>(handle));
			return this->asset_storage[handle_val].get();
		}

		Asset* get(AssetHandle handle)
		{
			if(handle == tz::nullhand) return nullptr;
			std::size_t handle_val = static_cast<std::size_t>(static_cast<tz::hanval>(handle));
			return this->asset_storage[handle_val].get();
		}

		void set(AssetHandle handle, Asset* value)
		{
			std::size_t handle_val = static_cast<std::size_t>(static_cast<tz::hanval>(handle));
			tz::assert(!this->asset_storage[handle_val].owning(), "AssetStorageCommon: Try to set specific asset value, but the asset at that handle is not a reference (it is owned by us)");
			this->asset_storage[handle_val] = value;
		}
	private:
		std::vector<maybe_owned_ptr<Asset>> asset_storage;
	};

	/**
	 * @ingroup tz_gl2_renderer
	 * Helper function which displays render-api-agnostic information about renderers.
	 */
	void common_renderer_dbgui(renderer_type auto& renderer)
	{
		if(renderer.is_null())
		{
			ImGui::Text("Null renderer");
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
				ImGui::Text("resource Count: %u", renderer.resource_count());
				static int res_id = 0;
				res_id = std::clamp(res_id, 0, static_cast<int>(rcount));
				ImGui::SliderInt("resource ID:", &res_id, 0, rcount);

				// Display information about current resource.
				ImGui::Indent();
				renderer.get_resource(static_cast<tz::hanval>(res_id))->dbgui();
				ImGui::Unindent();
			}
		}
		else
		{
			ImGui::Text("renderer has no resources.");
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
					ImGui::Text("[resource %zu]", static_cast<std::size_t>(static_cast<tz::hanval>(han)));
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
					ImGui::Text("[resource %zu]", static_cast<std::size_t>(static_cast<tz::hanval>(han)));
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
					renderer.edit
					(
						tz::gl::RendererEditBuilder{}
						.render_state
						({
							.clear_colour = col
						})
						.build()
					);
				}
			}
			// Graphics - Tri Count
			{
				if(renderer.get_state().graphics.draw_buffer != tz::nullhand)
				{
					ImGui::Text("Triangle Count: Indirect Buffer");
				}
				else
				{
					ImGui::Text("Triangle Count: %zu", renderer.get_state().graphics.tri_count);
				}
			}
			// Graphics - Wireframe Mode
			{
				bool wfm = renderer.get_state().graphics.wireframe_mode;
				if(ImGui::Checkbox("Wireframe Mode", &wfm))
				{
					renderer.edit
					(
						tz::gl::RendererEditBuilder{}
						.render_state
						({
							.wireframe_mode = wfm
						})
						.build()
					);
				}
			}
			ImGui::Unindent();
			ImGui::TextColored(ImVec4{1.0f, 0.6f, 0.6f, 1.0f}, "Compute");
			ImGui::Indent();
			// Compute - Kernel
			{
				auto kern = static_cast<tz::vec3i>(renderer.get_state().compute.kernel);
				if(ImGui::DragInt3("Kernel", kern.data().data(), 0.25f, 0, 64))
				{
					renderer.edit
					(
						tz::gl::RendererEditBuilder{}
						.compute
						({
							.kernel = kern
						})
						.build()
					);
				}
			}
			ImGui::Unindent();
		}
		if(!renderer.get_options().empty() && ImGui::CollapsingHeader("Renderer Options"))
		{
			for(renderer_option option : renderer.get_options())
			{
				ImGui::Text("%s", detail::renderer_option_strings[static_cast<int>(option)]);
			}
			#if TZ_DEBUG
				ImGui::PushTextWrapPos();
				ImGui::TextDisabled("Note: In debug builds, extra options might be present that you did not ask for. These are added to allow the debug-ui to display ontop of your rendered output.");
				ImGui::PopTextWrapPos();
			#endif //TZ_DEBUG
		}
		ImGui::Separator();
		ImGui::PopID();
	}

}

#endif // TOPAZ_GL2_IMPL_FRONTEND_COMMON_RENDERER_HPP
