#ifndef TOPAZ_GL_IMGUI_TZGLP_PREVIEW_HPP
#define TOPAZ_GL_IMGUI_TZGLP_PREVIEW_HPP
#include "gl/tz_imgui/imgui_context.hpp"
#include "gl/shader_preprocessor.hpp"
#include "gl/object.hpp"
#include <unordered_map>

namespace tz::ext::imgui::gl
{
	enum class SupportedModule : int
	{
		SSBOModule = 0,
		UBOModule = 1,
		BindlessSamplerModule = 2,
		Count = 3,
	};

	inline constexpr const char* get_supported_module_name(SupportedModule module)
	{
		switch(module)
		{
			case SupportedModule::SSBOModule:
				return "SSBO Module";
			break;
			case SupportedModule::UBOModule:
				return "UBO Module";
			break;
			case SupportedModule::BindlessSamplerModule:
				return "Bindless Sampler Module";
			break;
			default:
				return "Unknown Module";
			break;
		}
	}

	class TZGLPPreview : public ImGuiWindow
	{
	public:
		TZGLPPreview();
		~TZGLPPreview();
		std::size_t add_preprocessor();
		virtual void render() override;
		template<typename ModuleT, typename... Args>
		void add_module(std::size_t preprocessor_id, Args&&... args)
		{
			this->preprocessors[preprocessor_id].emplace_module(std::forward<Args>(args)...);
		}
		std::string preprocess(std::size_t preprocessor_id, std::string source);
	private:
		void render_module_configuration(std::size_t preprocessor_id);

		std::vector<tz::gl::ShaderPreprocessor> preprocessors;
		std::unordered_map<std::size_t, std::vector<SupportedModule>> preprocessor_modules;
		bool show_module_configuration;
		tz::gl::Object* dummy_object;
	};
}

#endif // TOPAZ_GL_IMGUI_TZGLP_PREVIEW_HPP