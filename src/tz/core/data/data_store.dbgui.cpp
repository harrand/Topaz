#include "tz/core/data/data_store.hpp"
#include "imgui.h"
#include "tz/dbgui/dbgui.hpp"

namespace tz
{
	void dbgui_data_store_value(data_store* ds, std::string key, data_store_value val)
	{
		std::visit([&ds, &key](auto&& arg)
		{
			using T = std::decay_t<decltype(arg)>;
			T v = arg;
			auto set = [&ds, &key, &v]()
			{
				ds->set
				({
					.key = key,
					.val = v
				});
			};
			if constexpr(std::is_same_v<T, bool>)
			{
				if(ImGui::Checkbox("##bool", &v))
				{
					set();
				}
			}
			else if constexpr(std::is_same_v<T, float>)
			{
				if(ImGui::InputFloat("##float", &v, 0.0f, 0.0f, "%g", ImGuiInputTextFlags_EnterReturnsTrue))
				{
					set();
				}
			}
			else if constexpr(std::is_same_v<T, double>)
			{
				if(ImGui::InputDouble("##double", &v, 0.0, 0.0, "%g", ImGuiInputTextFlags_EnterReturnsTrue))
				{
					set();
				}
			}
			else if constexpr(std::is_same_v<T, int>)
			{
				if(ImGui::InputInt("##int", &v, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					set();
				}
			}
			else if constexpr(std::is_same_v<T, unsigned int>)
			{
				if(ImGui::InputScalar("##uint", ImGuiDataType_U32, &v, 0, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					set();
				}
			}
			else if constexpr(std::is_same_v<T, std::string>)
			{
				if(ImGui::InputText("##str", &v, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					set();
				}
			}
		}, val);
	}

	void data_store::dbgui()
	{
		using store_t = decltype(this->store);
		// retrieve a copy for minimal lock contention.
		store_t store_cpy;
		{
			std::shared_lock<mutex> slock(this->mtx);
			store_cpy = this->store;
		}
		ImGui::Text("%zu entries", store_cpy.size());
		if(ImGui::Button("Clear"))
		{
			this->clear();
		}
		ImGui::Separator();
		if(ImGui::BeginTable("datastore", 2))
		{
			for(const auto& [key, val] : store_cpy)
			{
				int hash = std::hash<std::string>{}(key);
				ImGui::PushID(hash);
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				if(ImGui::Button("x"))
				{
					this->remove({.key = key});
					ImGui::PopID();
					break;
				}
				ImGui::SameLine();
				if(!std::holds_alternative<std::nullptr_t>(val) && ImGui::Button("o"))
				{
					this->set({.key = key, .val = nullptr});
				}
				ImGui::SameLine();
				ImGui::Text("%s", key.c_str());
				ImGui::TableNextColumn();
				dbgui_data_store_value(this, key, val);
				ImGui::PopID();
			}
			ImGui::EndTable();
		}
	}
}