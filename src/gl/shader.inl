namespace tz::gl
{
	template<ShaderType Type>
	void ShaderProgram::set(std::unique_ptr<Shader> shader)
	{
		constexpr std::size_t idx = static_cast<std::size_t>(Type);
		static_assert(Type != ShaderType::NUM_TYPES, "tz::gl::ShaderProgram::set<Type>(...): ShaderType::NUM_TYPES is not a valid parameter here.");
		this->shaders[idx] = std::move(shader);
	}
}