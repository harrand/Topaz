#include "gl/shader_preprocessor.hpp"
#include "core/debug/assert.hpp"

namespace tz::gl
{
	ShaderPreprocessor::ShaderPreprocessor(std::string source): source(source){}

	std::size_t ShaderPreprocessor::size() const
	{
		return this->modules.size();
	}

	bool ShaderPreprocessor::empty() const
	{
		return this->modules.empty();
	}

	void ShaderPreprocessor::preprocess()
	{
		for(const auto& module_ptr : this->modules)
			module_ptr->operator()(this->source);
	}

	const std::string& ShaderPreprocessor::result() const
	{
		return this->source;
	}

	p::IModule* ShaderPreprocessor::operator[](std::size_t idx)
	{
		topaz_assert(idx < this->size(), "tz::gl::ShaderPreprocessor[", idx, "]: Index ", idx, " is out of range! Size: ", this->size());
		return this->modules[idx].get();
	}

	const p::IModule* ShaderPreprocessor::operator[](std::size_t idx) const
	{
		topaz_assert(idx < this->size(), "tz::gl::ShaderPreprocessor[", idx, "]: Index ", idx, " is out of range! Size: ", this->size());
		return this->modules[idx].get();
	}
}

namespace tz::gl::p
{
	IncludeModule::IncludeModule(std::string source_path): path(source_path){}

	void IncludeModule::operator()(std::string& source) const
	{
		// TODO: Do include magic!
	}
}