#ifndef TOPAZ_GL_API_COMPONENT_HPP
#define TOPAZ_GL_API_COMPONENT_HPP
#include "gl/api/resource.hpp"

namespace tz::gl
{
	class IComponent
	{
	public:
		  virtual ~IComponent() = default;
		  virtual const IResource* get_resource() const = 0;
		  virtual IResource* get_resource() = 0;
		  bool has_resource() const
		  {
			  return this->get_resource() != nullptr;
		  }
	};
}

#endif // TOPAZ_GL_API_COMPONENT_HPP