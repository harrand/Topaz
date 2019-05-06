#include "platform_specific/shader_program.hpp"

#ifdef TOPAZ_OPENGL
namespace tz::platform
{

	template<class T>
	Uniform<T>::Uniform(const OGLShaderProgram* shader_program, std::string uniform_location, T value): UniformImplicit(shader_program, uniform_location), value(value), uniform_handle(this->shader_program->get_uniform_location(this->uniform_location).value_or(-1)){}

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
	std::unique_ptr<UniformImplicit> Uniform<T>::partial_unique_clone() const
	{
		return std::unique_ptr<Uniform<T>>{new Uniform<T>(*this)};
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
	Uniform<T>::Uniform(const Uniform<T>& copy): UniformImplicit(0, copy.uniform_location), value(copy.value), uniform_handle(-1){}

	template<class T>
	void Uniform<T>::retarget(const tz::platform::OGLShaderProgram *program)
	{
		UniformImplicit::retarget(program);
		this->uniform_handle = program->get_uniform_location(this->uniform_location).value_or(-1);
	}
}
#endif