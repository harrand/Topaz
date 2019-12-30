#ifndef TOPAZ_GL_SHADER_PREPROCESSOR_HPP
#define TOPAZ_GL_SHADER_PREPROCESSOR_HPP
#include <string>
#include <vector>
#include <memory>

namespace tz::gl
{
	namespace p
	{
		class IModule
		{
		public:
			virtual void operator()(std::string& source) const = 0;
		};
	}

	class ShaderPreprocessor
	{
	public:
		ShaderPreprocessor(std::string source);
		std::size_t size() const;
		bool empty() const;
		template<typename ModuleT, typename... Args>
		std::size_t emplace_module(Args&&... args);
		void preprocess();
		const std::string& result() const;
		p::IModule* operator[](std::size_t idx);
		const p::IModule* operator[](std::size_t idx) const;
	private:
		std::string source;
		std::vector<std::unique_ptr<p::IModule>> modules;
	};
}

namespace tz::gl::p
{
	// Modules!
	class IncludeModule : public IModule
	{
	public:
		IncludeModule(std::string source_path);
		virtual void operator()(std::string& source) const override;
	private:
		std::string path;
	};
}

#include "gl/shader_preprocessor.inl"
#endif // TOPAZ_GL_SHADER_PREPROCESSOR_HPP