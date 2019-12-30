#ifndef TOPAZ_GL_SHADER_PREPROCESSOR_HPP
#define TOPAZ_GL_SHADER_PREPROCESSOR_HPP
#include <string>
#include <vector>
#include <memory>

namespace tz::gl
{
	namespace p
	{
		/**
		 * Interface for a ShaderPreprocessor Module.
		 * 
		 * Modules are responsible for transforming a specific preprocessor directive or another specific task.
		 * Examples:
		 * - Handling #include
		 * - Handling #static_print
		 * - Handling #static_assert (This will require additional ShaderCompiler extension though...)
		 * 
		 */
		class IModule
		{
		public:
			/**
			 * Invoke the Module, performing source transformation in-place.
			 */
			virtual void operator()(std::string& source) const = 0;
		};
	}

	/**
	 * ShaderPreprocessors are executed on Shader component source-code prior to upload and compilation.
	 * 
	 * The idea is to populate a ShaderPreprocessors with Modules offering features that you want, and running it on the source before passing it to the graphics drivers for compilation.
	 * This allows us to have extremely useful interactivity between the Topaz engine runtime and the shaders that you have written.
	 * 
	 * Example: SSBOs are supported natively in GLSL. However, SSBO binding code is required on the C++ side to read/write to the SSBO and set it up properly...
	 * Using a ShaderPreprocessor, one could use a bespoke module to instead declare SSBOs in GLSL via a preprocessor directive. This allows the TZ runtime to automatically execute some binding-code, preventing you from having to write it!
	 * 
	 * See tz::gl::p::IModule and subclasses for features offered by modules. You're also encouraged to write your own! It's trivial to write a custom Module and emplace it into your ShaderPreprocessor.
	 */
	class ShaderPreprocessor
	{
	public:
		/**
		 * Construct a ShaderPreprocessor with a source code fragment to apply transformations to.
		 * @param source The source-code to transform.
		 */
		ShaderPreprocessor(std::string source);
		/**
		 * Retrieve the number of active modules.
		 * @return Number of modules used in this preprocessor.
		 */
		std::size_t size() const;
		/**
		 * Query as to whether there are any modules in this preprocessor.
		 * @return True if there are any modules. Otherwise false.
		 */
		bool empty() const;
		/**
		 * Construct a Module in-place. It will be the last module to run during preprocessing.
		 * 
		 * Note: To change the order of module execution, emplace the Module here and invoke swap_module using the resultant index with another index.
		 * @tparam ModuleT Module Type to construct.
		 * @tparam Args Types of arguments used to construct the ModuleT.
		 * @param args Values of arguments used to construct the ModuleT.
		 * @return The resultant index of the created ModuleT.
		 */
		template<typename ModuleT, typename... Args>
		std::size_t emplace_module(Args&&... args);
		/**
		 * Swap the indices of two existing Modules in the preprocessor.
		 * 
		 * Note: Modules are invoked in ascending order of index. Use this to swap the order of execution of Modules as you wish.
		 * Precondition: a < this-size() && b < this->size(). Otherwise this will assert and invoke UB.
		 */
		void swap_modules(std::size_t a, std::size_t b);
		/**
		 * Invoke all Modules on the source-fragment.
		 * 
		 * Note: This will update the internal source-fragment. Invoking preprocess will simulate a second preprocessor pass. This may or may not be desirable.
		 * Note: The transformed source-fragment will become available in this->result().
		 */
		void preprocess();
		/**
		 * Retrieve the transformed source-fragment.
		 * 
		 * Note: It is not an error to invoke this before preprocess. In which case this will simply return the source unchanged.
		 * @return The initial source-fragment, post any transformations applied by preprocessing invocations.
		 */
		const std::string& result() const;
		/**
		 * Retrieve a pointer to an existing Module based upon its index.
		 * 
		 * Precondition: idx < this->size(). Otherwise, this will assert and invoke UB.
		 */
		p::IModule* operator[](std::size_t idx);
		/**
		 * Retrieve a pointer to an existing Module based upon its index.
		 * 
		 * Precondition: idx < this->size(). Otherwise, this will assert and invoke UB.
		 */
		const p::IModule* operator[](std::size_t idx) const;
	private:
		std::string source;
		std::vector<std::unique_ptr<p::IModule>> modules;
	};
}

namespace tz::gl::p
{
	// Modules!
	class IncludeModule : public IModule
	{
	public:
		IncludeModule(std::string source_path);
		virtual void operator()(std::string& source) const override;
	private:
		std::string path;
	};
}

#include "gl/shader_preprocessor.inl"
#endif // TOPAZ_GL_SHADER_PREPROCESSOR_HPP