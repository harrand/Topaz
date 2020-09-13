#include "gl/shader_preprocessor.hpp"
#include "gl/object.hpp"
#include "core/debug/assert.hpp"

namespace tz::gl
{
	namespace p
	{
		ObjectAwareModule::ObjectAwareModule(tz::gl::Object* o): o(o){}
	}

	ShaderPreprocessor::ShaderPreprocessor(std::string source): source(source){}

	std::size_t ShaderPreprocessor::size() const
	{
		return this->modules.size();
	}

	bool ShaderPreprocessor::empty() const
	{
		return this->modules.empty();
	}

	void ShaderPreprocessor::swap_modules(std::size_t a, std::size_t b)
	{
		topaz_assert(a < this->size() && b < this->size(), "tz::gl::ShaderPreprocessor::swap_modules(", a, ", ", b, "): One of the indices (", a, " & ", b, ") was out of range! Size: ", this->size());
		std::swap(this->modules[a], this->modules[b]);
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

	void ShaderPreprocessor::set_source(std::string source)
	{
		this->source = source;
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