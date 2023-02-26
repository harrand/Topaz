#include "imgui.h"
#undef assert
namespace tz::gl
{
	template<renderer_type R>
	const R& device_common<R>::get_renderer(tz::gl::renderer_handle handle) const
	{
		return this->renderers[static_cast<std::size_t>(static_cast<tz::hanval>(handle))];
	}

	template<renderer_type R>
	R& device_common<R>::get_renderer(tz::gl::renderer_handle handle)
	{
		return this->renderers[static_cast<std::size_t>(static_cast<tz::hanval>(handle))];
	}

	template<renderer_type R>
	void device_common<R>::destroy_renderer(tz::gl::renderer_handle handle)
	{
		std::size_t h = static_cast<std::size_t>(static_cast<tz::hanval>(handle));
#if TZ_DEBUG
		bool free_list_contains = std::find(this->free_list.begin(), this->free_list.end(), h) != this->free_list.end();
		tz::assert(!free_list_contains, "Detected double-destroy of renderer of handle value %zu", h);
		tz::assert(this->renderers.size() > h, "Detected attempted destroy of renderer of invalid handle value %zu. device renderer storage does not have the capacity for this, meaning no renderer with this handle was ever returned by this device.", h);

#endif // TZ_DEBUG
		this->renderers[h] = R::null();
		this->free_list.push_back(h);
	}

	template<renderer_type R>
	std::size_t device_common<R>::renderer_count() const
	{
		return this->renderers.size() - this->free_list.size();
	}

	template<renderer_type R>
	template<typename... args>
	tz::gl::renderer_handle device_common<R>::emplace_renderer(args&&... argvals)
	{
		// If free list is empty, we need to expand our storage and retrieve a new handle.
		if(this->free_list.empty())
		{
			this->renderers.emplace_back(std::forward<args>(argvals)...);
			return static_cast<tz::hanval>(this->renderers.size() - 1);
		}
		else
		{
			// We destroyed a renderer in the past and now the free list contains a reference to the null renderer at its place. We can re-use this position.
			// If we destroyed a renderer in the past, let's re-use its handle. The renderer at the position will be a null renderer.
			tz::gl::renderer_handle h = static_cast<tz::hanval>(this->free_list.back());
			this->free_list.pop_back();
			this->get_renderer(h) = R{std::forward<args>(argvals)...};
			return h;
		}
	}

	template<renderer_type R>
	void device_common<R>::internal_clear()
	{
		this->renderers.clear();
	}

	template<tz::gl::device_type<tz::gl::renderer_info> T>
	void common_device_dbgui(T& device)
	{
		const std::size_t renderer_count = device.renderer_count();

		ImGui::TextColored(ImVec4{1.0f, 0.6f, 0.6f, 1.0f}, "tz::gl::get_device()");
		ImGui::Separator();
		ImGui::TextColored(ImVec4{1.0f, 0.6f, 0.6f, 1.0f}, "Summary");
		ImGui::Text("- The device currently stores %zu renderers", renderer_count);
		ImGui::Text("- The image_format of the window is %s", detail::image_format_strings[static_cast<int>(device.get_window_format())]);
		ImGui::Separator();
		ImGui::TextColored(ImVec4{1.0f, 0.6f, 0.6f, 1.0f}, "Renderers");
		static bool display_internal_renderers = false;
		ImGui::Checkbox("Display Internal Renderers", &display_internal_renderers);
		static int id = 0;
		if(renderer_count == 0)
		{
			return;
		}
		ImGui::SliderInt("renderer ID", &id, 0, renderer_count - 1);
		ImGui::Indent();
		const auto& renderer = device.get_renderer(static_cast<tz::hanval>(id));
		if(renderer.get_options().contains(tz::gl::renderer_option::_internal) && !display_internal_renderers)
		{
			ImGui::Text("Internal renderer");
			ImGui::Spacing();
			if(id > 0)
			{
				if(ImGui::Button("<<")){while(device.get_renderer(static_cast<tz::hanval>(id)).get_options().contains(tz::gl::renderer_option::_internal) && id > 0){id--;}}
				ImGui::SameLine();
				if(ImGui::Button("Prev")){id--;}
			}
			if(std::cmp_less(id, (renderer_count - 1)))
			{
				ImGui::SameLine();
				if(ImGui::Button("Next")){id++;}
				ImGui::SameLine();
				if(ImGui::Button(">>")){while(device.get_renderer(static_cast<tz::hanval>(id)).get_options().contains(tz::gl::renderer_option::_internal) && std::cmp_less(id, (renderer_count - 1))){id++;}}
			}
		}
		else
		{
			device.get_renderer(static_cast<tz::hanval>(id)).dbgui();
		}
		ImGui::Unindent();
		ImGui::Separator();
	}
}
