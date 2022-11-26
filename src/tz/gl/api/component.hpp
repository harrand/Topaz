#ifndef TOPAZ_GL2_API_COMPONENT_HPP
#define TOPAZ_GL2_API_COMPONENT_HPP
#include "hdk/data/vector.hpp"
#include "tz/gl/api/resource.hpp"
#include "tz/gl/declare/image_format.hpp"

namespace tz::gl
{
	class IComponent
	{
	public:
		IComponent() = default;
		virtual ~IComponent() = default;
		virtual const IResource* get_resource() const = 0;
		virtual IResource* get_resource() = 0;
	};

	template<typename T>
	concept BufferComponentType = requires(T t, std::size_t sz)
	{
		requires std::derived_from<T, IComponent>;
		{t.size()} -> std::convertible_to<std::size_t>;
		{t.resize(sz)} -> std::same_as<void>;
	};
	
	template<typename T>
	concept ImageComponentType = requires(T t, hdk::vec2ui dims)
	{
		requires std::derived_from<T, IComponent>;
		{t.get_dimensions()[0]} -> std::convertible_to<unsigned int>;
		{t.get_dimensions()[1]} -> std::convertible_to<unsigned int>;
		{t.get_format()} -> std::convertible_to<ImageFormat>;
		{t.resize(dims)} -> std::same_as<void>;
	};
}

#endif // TOPAZ_GL2_API_COMPONENT_HPP
