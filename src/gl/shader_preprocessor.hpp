#ifndef TOPAZ_GL_SHADER_PREPROCESSOR_HPP
#define TOPAZ_GL_SHADER_PREPROCESSOR_HPP
#include <string>
#include <vector>
#include <memory>
#include <regex>

// Forward Declares
namespace tz
{
	namespace gl
	{
		class Object;
	}
}

namespace tz::gl
{
	/**
	 * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
	 * @{
	 */

	namespace src
	{
		/**
		 * Perform general-purpose source information in-place on the given string.
		 * This will search through the given source string using the regular expression. Where matches are found, the inner match (provided by capture groups in the regex) are given to the transform function before source transformation is performed.
		 * 
		 * The transformation function should have the following signature:
		 * std::string my_transform_function(IterT begin, IterT end).
		 * The type IterT although is always the same, there is no guarantee that it will not change in future Topaz versions. For this reason, it is recommended to take them in as 'auto's.
		 * Note: IterT is guaranteed to have the following traits:
		 * - Support for std::distance(IterT, IterT). Note that std::distance(begin, end) is guaranteed to be equal to the number of capture-groups in the regex.
		 * - IterT::operator*() will yield a const std::string&
		 * - IterT::operator++ and IterT::operator+(std::size_t) are provided.
		 * IterT is not guaranteed to fully satisfy any iterator-traits, only the traits listed above.
		 * 
		 * The transform function should return a string that is the result of the source transformation. There is no functionality to skip this process.
		 * 
		 * Example: Processing #includes:
		 * Pass GLSL shader component source into the string, and a regex like "#include \"(.+)\"". Note that the regex has one capture-group.
		 * In this case, the iterator pairs will have a distance of one, and the result will contain a path to a file to include.
		 * The source transformation function can process the include and return the contents of the file. This function will then continue and perform the source transformation as provided by the transform_function.
		 * 
		 * For more information, I recommend checking out tz::gl::p::IncludeModule in gl/modules/include.hpp. This can make TZGLP modules incredibly simple.
		 * @tparam Runnable Type representing some callable function with the expected signature. Read above for more information on the expected signature. Note that the signature is expected to vary depending on the regex.
		 * @param source Source code to perform transformations on.
		 * @param reg Regular Expression used to search against the source-code.
		 * @param transform_function Function invoked with inner matches prior to each source transformation.
		 */
		template<typename Runnable>
		void transform(std::string& source, std::regex reg, Runnable transform_function);
	}

	namespace p
	{
		/**
		 * Interface for a ShaderPreprocessor Module.
		 * 
		 * Modules are responsible for transforming a specific preprocessor directive or another specific task.
		 * Examples:
		 * - Handling #include
		 * - Handling #static_print
		 * - Support for string literals in GLSL.
		 */
		class IModule
		{
		public:
			/**
			 * Invoke the Module, performing source transformation in-place.
			 */
			virtual void operator()(std::string& source) const = 0;
			virtual ~IModule() = default;
		};

		/**
		 * A specialised module which will always require the use of an existing tz::gl::Object.
		 */
		class ObjectAwareModule : public IModule
		{
		public:
			/**
			 * Construct the module with the given Object.
			 * 
			 * Precondition: The Object referenced by o must remain valid for the entirety of this module's lifetime. Otherwise, this will invoke UB.
			 */
			ObjectAwareModule(tz::gl::Object& o);
		protected:
			tz::gl::Object* o;
		};
	}

	/**
	 * \addtogroup tz_gl_p tz::gl Shader Preprocessing Module (TZGLP)
	 * @{
	 */

	/**
	 * ShaderPreprocessor is the basis of TZGLP.
	 * 
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
		ShaderPreprocessor(std::string source = "");
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
		 * TODO: Document
		 */
		void set_source(std::string source);
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

	/**
	 * @}
	 */

	/**
	 * @}
	 */
}

#include "gl/shader_preprocessor.inl"
#endif // TOPAZ_GL_SHADER_PREPROCESSOR_HPP