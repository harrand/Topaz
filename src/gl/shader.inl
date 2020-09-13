#include "core/debug/assert.hpp"

namespace tz::gl
{

	template<typename... Args>
	Shader* ShaderProgram::emplace(ShaderType type, Args&&... args)
	{
		std::size_t idx = static_cast<std::size_t>(type);
		topaz_assert(type != ShaderType::NUM_TYPES, "tz::gl::ShaderProgram::emplace<Type, Args...>(...): ShaderType::NUM_TYPES is not a valid parameter here.");
		this->shaders[idx] = Shader{type, std::forward<Args>(args)...};
		Shader* result = &this->shaders[idx].value();
		glAttachShader(this->handle, result->handle);
		return result;
	}
}