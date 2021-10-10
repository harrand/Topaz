#if TZ_VULKAN
#include "gl/impl/backend/vk/attribute_binding_description.hpp"

namespace tz::gl::vk
{
	VkVertexInputAttributeDescription VertexAttributeDescription::native() const
	{
		return this->desc;
	}
}

#endif // TZ_VULKAN