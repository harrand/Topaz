#include <type_traits>
#include "core/debug/assert.hpp"

namespace tz::gl
{

	template<PixelComponent Component>
	const Component& PixelRGBA<Component>::operator[](std::size_t idx) const
	{
		switch(idx)
		{
			case 0:
				return this->r;
			case 1:
				return this->g;
			case 2:
				return this->b;
			case 3:
				return this->a;
			default:
				topaz_assert(false, "PixelRGBA::operator[", idx, "] const. Value out of range!");
		}
		return this->r;
	}

	template<PixelComponent Component>
	Component& PixelRGBA<Component>::operator[](std::size_t idx)
	{
		switch(idx)
		{
			case 0:
				return this->r;
			case 1:
				return this->g;
			case 2:
				return this->b;
			case 3:
				return this->a;
			default:
				topaz_assert(false, "PixelRGBA::operator[", idx, "]. Value out of range!");
		}
		return this->r;
	}

	template<PixelComponent Component>
	bool PixelRGBA<Component>::operator==(const PixelRGBA<Component>& rhs) const
	{
		return this->r == rhs.r && this->g == rhs.g && this->b == rhs.b && this->a == rhs.a;
	}

	template<PixelComponent Component>
	bool PixelRGBA<Component>::operator!=(const PixelRGBA<Component>& rhs) const
	{
		return this->r != rhs.r || this->g != rhs.g || this->b != rhs.b || this->a != rhs.a;
	}

	template<PixelComponent Component>
	bool PixelRGB<Component>::operator==(const PixelRGB<Component>& rhs) const
	{
		return this->r == rhs.r && this->g == rhs.g && this->b == rhs.b;
	}

	template<PixelComponent Component>
	bool PixelRGB<Component>::operator!=(const PixelRGB<Component>& rhs) const
	{
		return this->r != rhs.r || this->g != rhs.g || this->b != rhs.b;
	}

	template<PixelComponent Component>
	const Component& PixelRGB<Component>::operator[](std::size_t idx) const
	{
		switch(idx)
		{
			case 0:
				return this->r;
			case 1:
				return this->g;
			case 2:
				return this->b;
			default:
				topaz_assert(false, "PixelRGB::operator[", idx, "]: Index out of range!");
		}
		return this->r;
	}

	template<PixelComponent Component>
	Component& PixelRGB<Component>::operator[](std::size_t idx)
	{
		switch(idx)
		{
			case 0:
				return this->r;
			case 1:
				return this->g;
			case 2:
				return this->b;
			default:
				topaz_assert(false, "PixelRGB::operator[", idx, "]: Index out of range!");
		}
		return this->r;
	}

	template<PixelComponent Component>
	bool PixelRG<Component>::operator==(const PixelRG<Component>& rhs) const
	{
		return this->x == rhs.x && this->y == rhs.y;
	}

	template<PixelComponent Component>
	bool PixelRG<Component>::operator!=(const PixelRG<Component>& rhs) const
	{
		return this->x != rhs.x || this->y != rhs.y;
	}

	template<PixelComponent Component>
	const Component& PixelRG<Component>::operator[](std::size_t idx) const
	{
		switch(idx)
		{
			case 0:
				return this->x;
			case 1:
				return this->y;
			default:
				topaz_assert(false, "PixelRGB::operator[", idx, "]: Index out of range!");
		}
		return this->x;
	}

	template<PixelComponent Component>
	Component& PixelRG<Component>::operator[](std::size_t idx)
	{
		switch(idx)
		{
			case 0:
				return this->x;
			case 1:
				return this->y;
			default:
				topaz_assert(false, "PixelRGB::operator[", idx, "]: Index out of range!");
		}
		return this->x;
	}

	template<PixelComponent Component>
	bool PixelGrayscale<Component>::operator==(const PixelGrayscale<Component>& rhs) const
	{
		return this->c == rhs.c;
	}

	template<PixelComponent Component>
	bool PixelGrayscale<Component>::operator!=(const PixelGrayscale<Component>& rhs) const
	{
		return this->c != rhs.c;
	}

	template<PixelComponent Component>
	const Component& PixelGrayscale<Component>::operator[](std::size_t idx) const
	{
		switch(idx)
		{
			case 0:
				return this->c;
			default:
				topaz_assert(false, "PixelGrayscale::operator[", idx, "]: Index out of range!");
		}
		return this->c;
	}

	namespace pixel
	{
		template<PixelComponent Component>
		constexpr GLenum parse_component_type()
		{
			#define chk(x, y) std::is_same_v<x, y>
			if constexpr(chk(Component, unsigned char) || chk(Component, std::byte))
			{
				return GL_UNSIGNED_BYTE;
			}
			else if constexpr(chk(Component, char))
			{
				return GL_BYTE;
			}
			else if constexpr(chk(Component, unsigned short))
			{
				return GL_UNSIGNED_SHORT;
			}
			else if constexpr(chk(Component, unsigned int))
			{
				return GL_UNSIGNED_INT;
			}
			else if constexpr(chk(Component, int))
			{
				return GL_INT;
			}
			else if constexpr(chk(Component, float))
			{
				return GL_FLOAT;
			}
			else
			{
				return GL_INVALID_VALUE;
			}
		}

		template<PixelType PixelT>
		constexpr GLint parse_internal_format()
		{
			#define chk(x, y) std::is_same_v<x, y>
			using ComponentType = PixelT::ComponentType;
			constexpr GLenum comp_type = tz::gl::pixel::parse_component_type<ComponentType>();
			if constexpr(chk(PixelT, PixelRGBA<ComponentType>))
			{
				// Is some form of RGBA...
				// Let's find out specifically which!
				switch(comp_type)
				{
					case GL_UNSIGNED_BYTE:
					case GL_BYTE:  
						return GL_RGBA8;
						break;
				}
			}
			else if constexpr(chk(PixelT, PixelRGB<ComponentType>))
			{
				// If some form of RGB...
				switch(comp_type)
				{
					case GL_UNSIGNED_BYTE:
					case GL_BYTE:
						return GL_RGB8;
						break;
				}
			}
			else if constexpr(chk(PixelT, PixelRG<ComponentType>))
			{
				// If some form of XY...
				switch(comp_type)
				{
					case GL_UNSIGNED_BYTE:
					case GL_BYTE:
						return GL_RG8;
						break;
				}
			}
			return GL_INVALID_VALUE;
		}

		template<PixelType PixelT>
		constexpr GLenum parse_format()
		{
			#define chk(x, y) std::is_same_v<x, y>
			using ComponentType = PixelT::ComponentType;
			if constexpr(chk(PixelT, PixelRGBA<ComponentType>))
			{
				return GL_RGBA;
			}
			else if constexpr(chk(PixelT, PixelRGB<ComponentType>))
			{
				return GL_RGB;
			}
			else if constexpr(chk(PixelT, PixelRG<ComponentType>))
			{
				return GL_RG;
			}
			else if constexpr(chk(PixelT, PixelGrayscale<ComponentType>))
			{
				return GL_DEPTH_COMPONENT;
			}
			return GL_INVALID_VALUE;
		}
	}
}