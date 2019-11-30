//
// Created by Harrand on 05/04/2019.
//

#include "gl/shader_program.hpp"

#ifdef TOPAZ_OPENGL
namespace tz::gl
{
	OGLShaderComponentCompileResult::OGLShaderComponentCompileResult(const OGLShaderComponent& component): success(false), error_message(std::nullopt)
	{
		GLint success = 0;
		// Query compile status
		glGetShaderiv(component.shader_handle, GL_COMPILE_STATUS, &success);
		if(success == GL_TRUE)
			this->success = true;
		else
		{
			// If we failed, try and retrieve an error message from the driver.
			this->success = false;
			std::string error_message_buffer;
			error_message_buffer.resize(4096);
			GLsizei actual_length;
			glGetShaderInfoLog(component.shader_handle, static_cast<GLsizei>(error_message_buffer.size()), &actual_length, error_message_buffer.data());
			error_message_buffer.resize(static_cast<std::size_t>(actual_length));
			this->error_message = std::move(error_message_buffer);
		}
	}

	bool OGLShaderComponentCompileResult::was_successful() const
	{
		return this->success;
	}

	const std::string* OGLShaderComponentCompileResult::get_error_message() const
	{
		if(this->error_message.has_value())
			return &this->error_message.value();
		else
			return nullptr;
	}

	void OGLShaderComponentCompileResult::report(std::ostream& str) const
	{
		str << "Compilation " << (this->success ? "Successful" : "Failed") << ". Message: " << this->error_message.value_or("None") << "\n";
	}

	void OGLShaderComponentCompileResult::report_if_fail(std::ostream& str) const
	{
		if(!this->success)
			this->report(str);
	}

	OGLShaderComponent::OGLShaderComponent(tz::gl::OGLShaderComponentType type, const std::optional<std::string>& source): shader_handle(0), type(type)
	{
		this->shader_handle = glCreateShader(static_cast<GLenum>(this->type));
		// If source is passed into constructor, upload and compile it instantly.
		if(source.has_value())
		{
			this->upload_source(source.value());
			this->compile();
		}
	}

	OGLShaderComponent::OGLShaderComponent(const OGLShaderComponent& copy): OGLShaderComponent(copy.type, std::nullopt)
	{
		GLint source_length;
		glGetShaderiv(copy.shader_handle, GL_SHADER_SOURCE_LENGTH, &source_length);
		if(source_length == 0) // If no code is in the parameter, then don't bother copying it.
			return;
		std::vector<GLchar> source_string;
		source_string.resize(static_cast<std::size_t>(source_length));
		glGetShaderSource(copy.shader_handle, source_length, nullptr, source_string.data());
		std::string source;
		for(GLchar glchar : source_string)
			source += static_cast<char>(glchar);
		this->upload_source(source);
		// If the parameter hasn't yet been successfully compiled, don't bother trying now.
		if(copy.get_compile_result().was_successful())
			this->compile();
	}

	OGLShaderComponent::OGLShaderComponent(OGLShaderComponent&& move): shader_handle(move.shader_handle), type(move.type)
	{
		move.shader_handle = 0;
	}

	OGLShaderComponent::~OGLShaderComponent()
	{
		glDeleteShader(this->shader_handle);
	}

	OGLShaderComponent& OGLShaderComponent::operator=(OGLShaderComponent rhs)
	{
		OGLShaderComponent::swap(*this, rhs);
		return *this;
	}

	const OGLShaderComponentType& OGLShaderComponent::get_type() const
	{
		return this->type;
	}

	void OGLShaderComponent::upload_source(std::string source) const
	{
        tz::debug::print("OGLShaderComponent::upload_source(string): Checking for headers...\n");
		auto length = static_cast<GLint>(source.length());
		const GLchar* source_data = source.c_str();
		glShaderSource(this->shader_handle, 1, &source_data, &length);
	}

	OGLShaderComponentCompileResult OGLShaderComponent::compile() const
	{
		glCompileShader(this->shader_handle);
		return this->get_compile_result();
	}

	OGLShaderComponentCompileResult OGLShaderComponent::get_compile_result() const
	{
		return {*this};
	}

	void OGLShaderComponent::swap(OGLShaderComponent& lhs, OGLShaderComponent& rhs)
	{
		std::swap(lhs.shader_handle, rhs.shader_handle);
		std::swap(lhs.type, rhs.type);
	}

	UniformState::UniformState(const OGLShaderProgram* target): data(), target(target) {}

	UniformState::UniformState(const UniformState& copy): data(), target(copy.target)
	{
		for(const auto&[location, uniform_ptr] : copy.data)
		{
			std::unique_ptr<UniformImplicit> clone = uniform_ptr->partial_unique_clone();
			clone->retarget(this->target);
			this->data[location] = std::move(clone);
		}
	}

	UniformState::UniformState(UniformState&& move): data(std::move(move.data)), target(move.target){}

	UniformState& UniformState::operator=(UniformState rhs)
	{
		UniformState::swap(*this, rhs);
		return *this;
	}

	std::size_t UniformState::size() const
	{
		return this->data.size();
	}

	void UniformState::clear()
	{
		this->data.clear();
	}

	bool UniformState::has_location(const std::string& location) const
	{
		return this->data.find(location) != this->data.cend();
	}

	void UniformState::retarget(const OGLShaderProgram* target)
	{
		this->target = target;
		for(auto& pair: this->data)
			pair.second->retarget(target);
	}

	void UniformState::push_all() const
	{
		for(auto& pair : this->data)
			pair.second->push();
	}

	void UniformState::swap(UniformState& lhs, UniformState& rhs)
	{
		std::swap(lhs.data, rhs.data);
		std::swap(lhs.target, rhs.target);
	}

	OGLShaderProgram::OGLShaderProgram(): program_handle(glCreateProgram()), components(), state(){}

	OGLShaderProgram::OGLShaderProgram(const OGLShaderProgram& copy): OGLShaderProgram()
	{
		for(auto& component_ptr : copy.components)
			if(component_ptr != nullptr)
				this->emplace_shader_component(*component_ptr);
		this->state = copy.state;
		this->state.retarget(this);
	}

	OGLShaderProgram::OGLShaderProgram(OGLShaderProgram&& move): program_handle(move.program_handle), components(std::move(move.components)), state(std::move(move.state))
	{
		move.program_handle = 0;
		components.clear();
		state.clear();
	}

	OGLShaderProgram::~OGLShaderProgram()
	{
		for(auto& component_ptr : this->components)
			glDetachShader(this->program_handle, component_ptr->shader_handle);
		glDeleteProgram(this->program_handle);
	}

	OGLShaderProgram& OGLShaderProgram::operator=(OGLShaderProgram rhs)
	{
		OGLShaderProgram::swap(*this, rhs);
		return *this;
	}

	bool OGLShaderProgram::get_can_tessellate() const
	{
		return this->get_shader_component(OGLShaderComponentType::TESSELLATION_CONTROL) != nullptr;
	}

	const OGLShaderComponent* OGLShaderProgram::get_shader_component(OGLShaderComponentType type) const
	{
		for(auto& component_ptr : this->components)
			if(component_ptr->get_type() == type)
				return component_ptr.get();
		return nullptr;
	}

	std::optional<UniformLocation> OGLShaderProgram::get_uniform_location(const std::string& uniform_name) const
	{
		if(this->get_link_result().was_successful())
			return glGetUniformLocation(this->program_handle, uniform_name.c_str());
		return std::nullopt; // Can't get uniforms if the shader isn't linked yet.
	}

	bool OGLShaderProgram::has_uniform(const std::string &uniform_name) const
	{
		return this->state.has_location(uniform_name);
	}

	void OGLShaderProgram::bind_attribute_location(GLuint index, const std::string& name) const
	{
		glBindAttribLocation(this->program_handle, index, name.c_str());
	}

	void OGLShaderProgram::bind_attribute(const OGLVertexAttribute& attribute, const std::string& name) const
	{
		this->bind_attribute_location(attribute.get_id(), name);
	}

	void OGLShaderProgram::bind_uniform_block(const OGLUniformBuffer& uniform_buffer, const std::string& name) const
	{
		GLuint block_index = glGetUniformBlockIndex(this->program_handle, name.c_str());
		glUniformBlockBinding(this->program_handle, block_index, uniform_buffer.binding_id);
	}

    void OGLShaderProgram::bind_shader_storage_block(const OGLShaderStorageBuffer& storage_buffer, const std::string& name) const
    {
        GLuint block_index = glGetProgramResourceIndex(this->program_handle, GL_SHADER_STORAGE_BLOCK, name.c_str());
        glShaderStorageBlockBinding(this->program_handle, block_index, storage_buffer.layout_qualifier_id);
    }


	bool OGLShaderProgram::is_fully_compiled() const
	{
		for(auto& component_ptr : this->components)
		{
			if(!component_ptr->get_compile_result().was_successful())
				return false;
		}
		return true;
	}

	OGLShaderProgramLinkResult OGLShaderProgram::link() const
	{
		if(!this->is_fully_compiled())
			return {};
		glLinkProgram(this->program_handle);
		return this->get_link_result();
	}

	OGLShaderProgramLinkResult OGLShaderProgram::get_link_result() const
	{
		if(!this->is_fully_compiled())
			return {};
		return {*this};
	}

	OGLShaderProgramValidateResult OGLShaderProgram::validate() const
	{
		if(!this->get_link_result().was_successful())
			return {};
		glValidateProgram(this->program_handle);
		return this->get_validate_result();
	}

	OGLShaderProgramValidateResult OGLShaderProgram::get_validate_result() const
	{
		if(!this->get_link_result().was_successful())
			return {};
		return {*this};
	}

	void OGLShaderProgram::set_output_feedback(const std::string& output_name, bool interleaved) const
	{
		const GLchar* outputs[] = {output_name.c_str()};
		glTransformFeedbackVaryings(this->program_handle, 1, outputs, interleaved ? GL_INTERLEAVED_ATTRIBS : GL_SEPARATE_ATTRIBS);
	}

	void OGLShaderProgram::bind() const
	{
		glUseProgram(this->program_handle);
	}

	void OGLShaderProgram::update()
	{
		this->state.push_all();
	}

	void OGLShaderProgram::swap(OGLShaderProgram& lhs, OGLShaderProgram& rhs)
	{
		std::swap(lhs.program_handle, rhs.program_handle);
		std::swap(lhs.components, rhs.components);
		std::swap(lhs.state, rhs.state);
	}

	OGLShaderProgramLinkResult::OGLShaderProgramLinkResult(const OGLShaderProgram& shader_program): was_attempted(true), success(false), error_message(std::nullopt)
	{
		GLint success = 0;
		glGetProgramiv(shader_program.program_handle, GL_LINK_STATUS, &success);
		if(success == GL_TRUE)
			this->success = true;
		else
		{
			this->success = false;
			std::string error_message_buffer;
			error_message_buffer.resize(4096);
			GLsizei actual_length;
			glGetProgramInfoLog(shader_program.program_handle, static_cast<GLsizei>(error_message_buffer.size()), &actual_length, error_message_buffer.data());
			error_message_buffer.resize(static_cast<std::size_t>(actual_length));
			this->error_message = std::move(error_message_buffer);
		}
	}

	OGLShaderProgramLinkResult::OGLShaderProgramLinkResult(): was_attempted(false), success(false), error_message("Not all components were previously compiled"){}

	bool OGLShaderProgramLinkResult::get_was_attempted() const
	{
		return this->was_attempted;
	}

	bool OGLShaderProgramLinkResult::was_successful() const
	{
		return this->success;
	}

	void OGLShaderProgramLinkResult::report(std::ostream& str) const
	{
		str << "Link " << (this->success ? "Successful" : "Failed") << ". Message: " << this->error_message.value_or("None") << "\n";
	}

	void OGLShaderProgramLinkResult::report_if_fail(std::ostream& str) const
	{
		if(!this->success)
			this->report(str);
	}

	OGLShaderProgramValidateResult::OGLShaderProgramValidateResult(const OGLShaderProgram &shader_program): was_attempted(true), success(false), error_message(std::nullopt)
	{
		GLint success = 0;
		glGetProgramiv(shader_program.program_handle, GL_VALIDATE_STATUS, &success);
		if(success == GL_TRUE)
			this->success = true;
		else
		{
			this->success = false;
			std::string error_message_buffer;
			error_message_buffer.resize(4096);
			GLsizei actual_length;
			glGetProgramInfoLog(shader_program.program_handle, static_cast<GLsizei>(error_message_buffer.size()), &actual_length, error_message_buffer.data());
			error_message_buffer.resize(static_cast<std::size_t>(actual_length));
			this->error_message = std::move(error_message_buffer);
		}
	}

	OGLShaderProgramValidateResult::OGLShaderProgramValidateResult(): was_attempted(false), success(false), error_message("The program has not been linked yet") {}

	bool OGLShaderProgramValidateResult::get_was_attempted() const
	{
		return this->was_attempted;
	}

	bool OGLShaderProgramValidateResult::was_successful() const
	{
		return this->success;
	}

	void OGLShaderProgramValidateResult::report(std::ostream& str) const
	{
		str << "Validate " << (this->success ? "Successful" : "Failed") << ". Message: " << this->error_message.value_or("None") << "\n";
	}

	void OGLShaderProgramValidateResult::report_if_fail(std::ostream& str) const
	{
		if(!this->success)
			this->report(str);
	}
}
#endif
