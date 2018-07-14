#include "data/matrix.hpp"

template<class T>
Uniform<T>::Uniform(GLuint shader_handle, std::string uniform_location, T value): UniformImplicit(shader_handle, uniform_location), value(value), uniform_handle(glGetUniformLocation(this->shader_handle, uniform_location.c_str())){}

template<class T>
const T& Uniform<T>::get_value() const
{
	return this->value;
}

template<class T>
void Uniform<T>::set_value(T value)
{
	this->value = value;
}

template<class T>
void Uniform<T>::push() const
{
	/*
	GLSL Standard for bools:
	"When the type of internal state is boolean, zero integer or floating-point values are converted to FALSE and non-zero values are converted to TRUE."
		scalars:
			bool
			int
			uint
			float
			double
		vectors:
			bvecn (bools)
			ivecn (signed integers)
			uvecn (unsigned integers)
			vecn (floats)
			dvecn (doubles)
		matrices (floating point):
			matnxm
			matn

		TZ Extras (handled by template specialisations):
	 	DirectionalLight(direction, colour, power)
	 	PointLight(position, colour, power)
	*/
	// Scalars
	if constexpr(std::is_same<decltype(this->value), bool>::value)
		glUniform1i(this->uniform_handle, this->value);
	else if constexpr(std::is_same<decltype(this->value), int>::value)
		glUniform1i(this->uniform_handle, this->value);
	else if constexpr(std::is_same<decltype(this->value), unsigned int>::value)
		glUniform1ui(this->uniform_handle, this->value);
	else if constexpr(std::is_same<decltype(this->value), float>::value)
		glUniform1f(this->uniform_handle, this->value);
	else if constexpr(std::is_same<decltype(this->value), double>::value)
		glUniform1d(this->uniform_handle, this->value);
	//Vectors (floating point only)
	else if constexpr(std::is_same<decltype(this->value), Vector2F>::value)
		glUniform2f(this->uniform_handle, this->value.x, this->value.y);
	else if constexpr(std::is_same<decltype(this->value), Vector3F>::value)
		glUniform3f(this->uniform_handle, this->value.x, this->value.y, this->value.z);
	else if constexpr(std::is_same<decltype(this->value), Vector4F>::value)
		glUniform4f(this->uniform_handle, this->value.x, this->value.y, this->value.z, this->value.w);
	//Matrices
	else if constexpr(std::is_same<decltype(this->value), Matrix2x2>::value)
		glUniformMatrix2fv(this->uniform_handle, 1, GL_TRUE, this->value.fill_data().data());
	else if constexpr(std::is_same<decltype(this->value), Matrix3x3>::value)
		glUniformMatrix3fv(this->uniform_handle, 1, GL_TRUE, this->value.fill_data().data());
	else if constexpr(std::is_same<decltype(this->value), Matrix4x4>::value)
		glUniformMatrix4fv(this->uniform_handle, 1, GL_TRUE, this->value.fill_data().data());
	else
		static_assert(std::is_void<decltype(this->value)>::value, "[Topaz Shader]: Uniform has unsupported type. Perhaps your desired version of OpenGL proceeds Topaz's too far by using newer types for uniforms?");
}

template<class T>
void Shader::add_uniform(Uniform<T>&& uniform)
{
	if(this->has_uniform(uniform.get_uniform_location()))
	{
		std::cerr << "[Shader]: Tried to add uniform with location '" << uniform.get_uniform_location() << "' to Shader with handle " << this->program_handle << ", of which the location is already occupied.\n";
		return;
	}
	if(this->uniform_counter >= tz::graphics::maximum_uniforms)
	{
		std::cerr << "[Shader]: Tried to add uniform with location '" << uniform.get_uniform_location() << "' to Shader with handle " << this->program_handle << ", but it already has the maximum number of uniforms attached (" << tz::graphics::maximum_uniforms << ").\n";
		return;
	}
	this->uniform_data[uniform_counter++] = std::make_unique<Uniform<T>>(std::forward<Uniform<T>>(uniform));
}

template<class T>
void Shader::emplace_uniform(std::string uniform_location, T value)
{
	this->add_uniform<T>(Uniform<T>(this->program_handle, uniform_location, value));
}

template<class T>
void Shader::set_uniform(std::string_view uniform_location, T value)
{
	if(!this->has_uniform(uniform_location))
	{
		this->emplace_uniform(std::string(uniform_location), value);
		return;
	}
	for(std::size_t i = 0; i < this->uniform_counter; i++)
		if(this->uniform_data[i]->get_uniform_location() == uniform_location)
			dynamic_cast<Uniform<T>*>(this->uniform_data[i].get())->set_value(value);
}

template<class T>
T Shader::get_uniform_value(std::string_view uniform_location) const
{
	if(!this->has_uniform(uniform_location))
	{
		std::cerr << "[Shader]: Tried to retrieve uniform_value for non-existent uniform. Returning default...\n";
		return T();
	}
	return dynamic_cast<Uniform<T>*>(this->get_uniform(uniform_location))->get_value();
}