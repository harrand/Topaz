#include "tz/gl/impl/vulkan/renderer2.hpp"

namespace tz::gl
{
	renderer_resource_manager::renderer_resource_manager(const tz::gl::renderer_info& rinfo):
	AssetStorageCommon<iresource>(rinfo.get_resources())
	{
		// AssetStorageCommon has a set of resources already from the rendere info.
		// However, if the renderer info contained resource references, the resource will be nullptr, and we will need to patch it up ourselves now.
		this->patch_resource_references(rinfo);
	}

	unsigned int renderer_resource_manager::resource_count() const
	{
		return AssetStorageCommon<iresource>::count();
	}

	const iresource* renderer_resource_manager::get_resource(tz::gl::resource_handle rh) const
	{
		return AssetStorageCommon<iresource>::get(rh);
	}

	iresource* renderer_resource_manager::get_resource(tz::gl::resource_handle rh)
	{
		return AssetStorageCommon<iresource>::get(rh);
	}

	const icomponent* renderer_resource_manager::get_component(tz::gl::resource_handle rh) const
	{
		return this->components[static_cast<std::size_t>(static_cast<tz::hanval>(rh))].get();
	}

	icomponent* renderer_resource_manager::get_component(tz::gl::resource_handle rh)
	{
		return this->components[static_cast<std::size_t>(static_cast<tz::hanval>(rh))].get();
	}

	void renderer_resource_manager::patch_resource_references(const tz::gl::renderer_info& rinfo)
	{
		this->components.reserve(this->resource_count());
		std::size_t resource_reference_counter = 0;
		for(std::size_t i = 0; i < this->resource_count(); i++)
		{
			tz::gl::resource_handle rh{static_cast<tz::hanval>(i)};
			iresource* res = this->get_resource(rh);
			// if resource is nullptr, then we get the next component in the list and patch that in.
			if(res == nullptr)
			{
				// find the component which should be in this resource slot.
				icomponent* comp = const_cast<icomponent*>(rinfo.get_components()[resource_reference_counter++]);
				// add the resource reference.
				this->components.push_back(comp);
				AssetStorageCommon<iresource>::set(rh, comp->get_resource());
			}
			else // otherwise, we create the component ourselves.
			{
				// make the component based on the resource.
				this->components.push_back(this->make_component_from(res));
			}
		}
	}

	tz::maybe_owned_ptr<icomponent> renderer_resource_manager::make_component_from(iresource* resource)
	{
		tz::assert(resource != nullptr);
		switch(resource->get_type())
		{
			case tz::gl::resource_type::buffer:
				return tz::make_owned<buffer_component_vulkan>(*resource);
			break;
			case tz::gl::resource_type::image:
				return tz::make_owned<image_component_vulkan>(*resource);
			break;
		}
		return nullptr;
	}

	renderer_vulkan2::renderer_vulkan2(const tz::gl::renderer_info& rinfo):
	renderer_resource_manager(rinfo),
	options(rinfo.get_options()),
	state(rinfo.state())
	{

	}

	const tz::gl::renderer_options& renderer_vulkan2::get_options() const
	{
		return this->options;
	}

	const tz::gl::render_state& renderer_vulkan2::get_state() const
	{
		return this->state;
	}
}
