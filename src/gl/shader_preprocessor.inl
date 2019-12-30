namespace tz::gl
{
	template<typename ModuleT, typename... Args>
	std::size_t ShaderPreprocessor::emplace_module(Args&&... args)
	{
		std::size_t idx = this->modules.size();
		this->modules.push_back(std::make_unique<ModuleT>(std::forward<Args>(args)...));
		return idx;
	}
}