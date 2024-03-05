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
				ImGui::SameLine(); ImGui::Text("bool");
			}
			else if constexpr(std::is_same_v<T, float>)
			{
				if(ImGui::InputFloat("##float", &v, 0.0f, 0.0f, "%g", ImGuiInputTextFlags_EnterReturnsTrue))
				{
					set();
				}
				ImGui::SameLine(); ImGui::Text("float");
			}
			else if constexpr(std::is_same_v<T, double>)
			{
				if(ImGui::InputDouble("##double", &v, 0.0, 0.0, "%g", ImGuiInputTextFlags_EnterReturnsTrue))
				{
					set();
				}
				ImGui::SameLine(); ImGui::Text("double");
			}
			else if constexpr(std::is_same_v<T, int>)
			{
				if(ImGui::InputInt("##int", &v, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					set();
				}
				ImGui::SameLine(); ImGui::Text("int");
			}
			else if constexpr(std::is_same_v<T, unsigned int>)
			{
				if(ImGui::InputScalar("##uint", ImGuiDataType_U32, &v, 0, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					set();
				}
				ImGui::SameLine(); ImGui::Text("uint");
			}
			else if constexpr(std::is_same_v<T, std::string>)
			{
				if(ImGui::InputText("##str", &v, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					set();
				}
				ImGui::SameLine(); ImGui::Text("string");
			}
		}, val);
	}

	void data_store_dbgui_new_row(data_store* ds)
	{
		static std::string key = "";
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("New Entry:");
		ImGui::SameLine();
		ImGui::InputText("##newkey", &key);
		ImGui::TableNextColumn();
		if(ImGui::Button("null"))
		{
			ds->add({.key = key, .val = nullptr});
			key = "";
		}
		ImGui::SameLine();
		if(ImGui::Button("bool"))
		{
			ds->add({.key = key, .val = false});
			key = "";
		}
		ImGui::SameLine();
		if(ImGui::Button("float"))
		{
			ds->add({.key = key, .val = 0.0f});
			key = "";
		}
		ImGui::SameLine();
		if(ImGui::Button("double"))
		{
			ds->add({.key = key, .val = 0.0});
			key = "";
		}
		ImGui::SameLine();
		if(ImGui::Button("int"))
		{
			ds->add({.key = key, .val = 0});
			key = "";
		}
		ImGui::SameLine();
		if(ImGui::Button("uint"))
		{
			ds->add({.key = key, .val = 0u});
			key = "";
		}
		ImGui::SameLine();
		if(ImGui::Button("string"))
		{
			ds->add({.key = key, .val = std::string{""}});
			key = "";
		}
		ImGui::SameLine();
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
		static std::string filter = "";
		if(ImGui::Button("x"))
		{
			filter.clear();
		}
		ImGui::SameLine();
		ImGui::InputText("Filter", &filter);
		ImGui::Separator();
		if(ImGui::BeginTable("datastore", 2))
		{
			for(const auto& [key, val] : store_cpy)
			{
				if(!filter.empty() && !key.starts_with(filter))
				{
					continue;
				}
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

				if(std::holds_alternative<std::nullptr_t>(val))
				{
					// is null. give it some initialisation options.
					if(ImGui::Button("float"))
					{
						this->set({.key = key, .val = 0.0f});
					}
					ImGui::SameLine();
					if(ImGui::Button("double"))
					{
						this->set({.key = key, .val = 0.0});
					}
					ImGui::SameLine();
					if(ImGui::Button("int"))
					{
						this->set({.key = key, .val = 0});
					}
					ImGui::SameLine();
					if(ImGui::Button("uint"))
					{
						this->set({.key = key, .val = 0u});
					}
					ImGui::SameLine();
					if(ImGui::Button("string"))
					{
						this->set({.key = key, .val = std::string{""}});
					}
				}
				else
				{
					dbgui_data_store_value(this, key, val);
				}
				ImGui::PopID();
			}
			data_store_dbgui_new_row(this);
			ImGui::EndTable();
		}
	}
}