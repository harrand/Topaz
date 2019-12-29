#include "core/debug/assert.hpp"

namespace tz::gl
{
	template<ShaderType Type>
	void ShaderProgram::set(std::unique_ptr<Shader> shader)
	{
		constexpr std::size_t idx = static_cast<std::size_t>(Type);
		static_assert(Type != ShaderType::NUM_TYPES, "tz::gl::ShaderProgram::set<Type>(...): ShaderType::NUM_TYPES is not a valid parameter here.");
		this->shaders[idx] = std::move(shader);
	}

	template<typename... Args>
	Shader* ShaderProgram::emplace(ShaderType type, Args&&... args)
	{
		std::size_t idx = static_cast<std::size_t>(type);
		topaz_assert(type != ShaderType::NUM_TYPES, "tz::gl::ShaderProgram::emplace<Type, Args...>(...): ShaderType::NUM_TYPES is not a valid parameter here.");
		this->shaders[idx] = std::make_unique<Shader>(type, std::forward<Args>(args)...);
		Shader* result = this->shaders[idx].get();
		glAttachShader(this->handle, result->handle);
		return result;
	}
}