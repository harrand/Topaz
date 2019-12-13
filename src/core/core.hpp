//
// Created by Harrand on 13/12/2019.
//

#ifndef TOPAZ_CORE_HPP
#define TOPAZ_CORE_HPP
#include "core/window.hpp"
#include <memory>

// Forward declare
namespace tz::ext::glfw
{
	class GLFWContext;
}

namespace tz::core
{
	class TopazCore
	{
	public:
		// Ignore ctor/dtor as we expect this to be a global.
		// Also break the standard and define as noexcept.
		TopazCore() noexcept;
		~TopazCore() = default;
		void initialise(const char* app_name);
		void terminate();

		bool is_initialised() const;
		const tz::ext::glfw::GLFWContext& context() const;
		const IWindow& window() const;
		IWindow& window();
	private:
		std::unique_ptr<IWindow> tz_window;
		bool initialised;
	};

	void initialise(const char* app_name);
	void update();
	void terminate();
	TopazCore& get();
}

#endif //TOPAZ_CORE_HPP
