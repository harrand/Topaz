#ifndef TOPAZ_GL2_API_COMPONENT_HPP
#define TOPAZ_GL2_API_COMPONENT_HPP
#include "tz/core/data/vector.hpp"
#include "tz/gl/api/resource.hpp"
#include "tz/gl/declare/image_format.hpp"

namespace tz::gl
{
	class icomponent
	{
	public:
		icomponent() = default;
		virtual ~icomponent() = default;
		virtual const iresource* get_resource() const = 0;
		virtual iresource* get_resource() = 0;
	};

	template<typename T>
	concept buffer_component_type = requires(T t, std::size_t sz)
	{
		requires std::derived_from<T, icomponent>;
		{t.size()} -> std::convertible_to<std::size_t>;
		{t.resize(sz)} -> std::same_as<void>;
	};
	
	template<typename T>
	concept image_component_type = requires(T t, tz::vec2ui dims)
	{
		requires std::derived_from<T, icomponent>;
		{t.get_dimensions()[0]} -> std::convertible_to<unsigned int>;
		{t.get_dimensions()[1]} -> std::convertible_to<unsigned int>;
		{t.get_format()} -> std::convertible_to<image_format>;
		{t.resize(dims)} -> std::same_as<void>;
	};
}

#endif // TOPAZ_GL2_API_COMPONENT_HPP
