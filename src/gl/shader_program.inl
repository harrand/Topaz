#include "gl/uniform.hpp"
#ifdef TOPAZ_OPENGL
namespace tz::gl
{
    template<typename T, typename... Args>
    Uniform<T>& UniformState::emplace(Args&&... args)
    {
        std::unique_ptr<Uniform<T>> uniform_ptr = std::make_unique<Uniform<T>>(std::forward<Args>(args)...);
        std::string location = uniform_ptr->get_uniform_location();
        this->data[location] = std::move(uniform_ptr);
        return dynamic_cast<Uniform<T>&>(*this->data[location]);
    }

    template<typename T>
    Uniform<T>* UniformState::at(const std::string& location)
    {
        Uniform<T>* ret = nullptr;
        try
        {
            ret = dynamic_cast<Uniform<T>*>(this->data.at(location).get());
        }catch(const std::out_of_range& oor){}
        return ret;
    }

    template<typename T>
    const Uniform<T>* UniformState::at(const std::string& location) const
    {
        const Uniform<T>* ret = nullptr;
        try
        {
            ret = dynamic_cast<const Uniform<T>*>(this->data.at(location).get());
        }catch(const std::out_of_range& oor){}
        return ret;
    }

	template<typename... Args>
	OGLShaderComponent& OGLShaderProgram::emplace_shader_component(Args&&... args)
	{
		this->components.push_back(std::make_unique<OGLShaderComponent>(std::forward<Args>(args)...));
		OGLShaderComponent& component = *this->components.back();
		glAttachShader(this->program_handle, component.shader_handle);
		return component;
	}

	template<typename T, typename... Args>
	Uniform<T>& OGLShaderProgram::emplace_uniform(Args&&... args)
	{
		return this->state.emplace<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	void OGLShaderProgram::set_uniform(const std::string& uniform_location, T value)
	{
		auto* ptr = this->state.at<T>(uniform_location);
		if(ptr != nullptr)
			ptr->set_value(value);
		else
			this->emplace_uniform<T>(this, uniform_location, value);
	}

	template<typename T>
	const T* OGLShaderProgram::get_uniform_value(const std::string& uniform_location) const
	{
		auto* ptr = this->state.at<T>(uniform_location);
		if(ptr != nullptr)
			return &ptr->get_value();
		else
			return nullptr;
	}
}

#endif