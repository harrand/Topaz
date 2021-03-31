#ifndef TOPAZ_GL_PIXEL_HPP
#define TOPAZ_GL_PIXEL_HPP
#include "glad/glad.h"
#include <cstddef>
#include <concepts>
#include <type_traits>

namespace tz::gl
{
	/**
	 * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
	 * @{
	 */

	/**
	 * PixelComponents must be PODs (Plain Old Data).
	 */
	template<typename ProspectiveComponent>
	concept PixelComponent = std::is_standard_layout_v<ProspectiveComponent> && std::is_trivial_v<ProspectiveComponent>;

	/**
	 * PixelTypes must contain:
	 * - the static member ::num_components -> std::size_t
	 * - the typedef ::ComponentType which satisfies the concept PixelComponent.
	 * - Must be equality-comparable with the same PixelType.
	 */
	template<typename ProspectivePixel>
	concept PixelType = requires
	{
		typename ProspectivePixel::ComponentType;
		{ProspectivePixel::num_components} -> std::convertible_to<std::size_t>;
		requires PixelComponent<typename ProspectivePixel::ComponentType>;
		requires std::equality_comparable<ProspectivePixel>;
	};

	template<PixelComponent Component>
	struct PixelRGBA
	{
		static constexpr std::size_t num_components = 4;
		using ComponentType = Component;
		Component r;
		Component g;
		Component b;
		Component a;
		const Component& operator[](std::size_t idx) const;
		Component& operator[](std::size_t idx);
		bool operator==(const PixelRGBA<Component>& rhs) const;
		bool operator!=(const PixelRGBA<Component>& rhs) const;
	};

	template<PixelComponent Component>
	struct PixelRGB
	{
		static constexpr std::size_t num_components = 3;
		using ComponentType = Component;
		Component r;
		Component g;
		Component b;
		const Component& operator[](std::size_t idx) const;
		Component& operator[](std::size_t idx);
		bool operator==(const PixelRGB<Component>& rhs) const;
		bool operator!=(const PixelRGB<Component>& rhs) const;
	};

	template<PixelComponent Component>
	struct PixelRG
	{
		static constexpr std::size_t num_components = 2;
		using ComponentType = Component;
		Component x;
		Component y;
		const Component& operator[](std::size_t idx) const;
		Component& operator[](std::size_t idx);
		bool operator==(const PixelRG<Component>& rhs) const;
		bool operator!=(const PixelRG<Component>& rhs) const;
	};

	template<PixelComponent Component>
	struct PixelGrayscale
	{
		static constexpr std::size_t num_components = 1;
		using ComponentType = Component;
		Component c;

		const Component& operator[](std::size_t idx) const;
		Component& operator[](std::size_t idx);
		bool operator==(const PixelGrayscale<Component>& rhs) const;
		bool operator!=(const PixelGrayscale<Component>& rhs) const;
	};

	namespace pixel
	{
		template<PixelComponent Component>
		constexpr GLenum parse_component_type();
		template<PixelType PixelT>
		constexpr GLint parse_internal_format();
		template<PixelType PixelT>
		constexpr GLenum parse_format();
	}

	using PixelRGBA8 = PixelRGBA<std::byte>;
	using PixelRGB8 = PixelRGB<std::byte>;
	using PixelRG8 = PixelRG<std::byte>;
	using PixelDepth = PixelGrayscale<float>;

	/**
	 * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
	 * @{
	 */
}

#include "gl/pixel.inl"
#endif // TOPAZ_GL_PIXEL_HPP