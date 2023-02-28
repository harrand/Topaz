#include "tz/gl/impl/vulkan/renderer2.hpp"
#include "tz/gl/device.hpp"

namespace tz::gl
{
//--------------------------------------------------------------------------------------------------
	renderer_resource_manager::renderer_resource_manager(const tz::gl::renderer_info& rinfo):
	AssetStorageCommon<iresource>(rinfo.get_resources())
	{
		this->patch_resource_references(rinfo);
		this->setup_dynamic_resource_spans();
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
		// AssetStorageCommon populates our set of resources already from the renderer info.
		// However, if the renderer info contained resource references (rinfo.ref_resource), the resource will be nullptr, and we will need to patch it up ourselves now.
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

	void renderer_resource_manager::setup_dynamic_resource_spans()
	{
		// go through each component. if its a dynamic resource, link the underlying image/buffer mapped data with the resource data.
		for(std::size_t i = 0; i < this->resource_count(); i++)
		{
			icomponent* comp = this->components[i].get();
			tz::assert(comp != nullptr);
			iresource* res = comp->get_resource();
			tz::assert(res != nullptr);
			if(res->get_access() == tz::gl::resource_access::static_fixed)
			{
				continue;
			}
			std::span<std::byte> underlying_component_data;
			switch(res->get_type())
			{
				case tz::gl::resource_type::buffer:
					underlying_component_data = static_cast<buffer_component_vulkan*>(comp)->vk_get_buffer().map_as<std::byte>();
				break;
				case tz::gl::resource_type::image:
					underlying_component_data = static_cast<image_component_vulkan*>(comp)->vk_get_image().map_as<std::byte>();
				break;
				default:
					tz::error("unrecognised resource_type");
				break;
			}
			auto resdata = res->data();
			tz::assert(resdata.size_bytes() <= underlying_component_data.size_bytes());
			std::copy(resdata.begin(), resdata.end(), underlying_component_data.begin());
			res->set_mapped_data(underlying_component_data);
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

//--------------------------------------------------------------------------------------------------

	renderer_descriptor_manager::renderer_descriptor_manager(const tz::gl::renderer_info& rinfo):
	renderer_resource_manager(rinfo)
	{
		this->deduce_descriptor_layout(rinfo.state());
	}

	void renderer_descriptor_manager::deduce_descriptor_layout(const tz::gl::render_state& state)
	{
		// figure out what descriptor layout we need, and set this->layout to it.
		// the old value of this->layout is discarded. it is an error to call this function while the layout is "in-use"
		std::size_t buffer_count = 0;
		std::size_t image_count = 0;
		for(std::size_t i = 0; i < renderer_resource_manager::resource_count(); i++)
		{
			const iresource* res = renderer_resource_manager::get_resource(static_cast<tz::hanval>(i));	
			tz::assert(res != nullptr);
			switch(res->get_type())
			{
				case tz::gl::resource_type::buffer:
					buffer_count++;
				break;
				case tz::gl::resource_type::image:
					image_count++;
				break;
			}
		}
		
		// now we know the number of buffers and resources, but we need a little bit of extra info.
		// what if one of the buffers is our index/draw-indirect buffer? if so, we need to exclude it from the descriptor layout, as it is not a shader resource.
		// this is why we need to know the render state.	
		if(state.graphics.index_buffer != tz::nullhand)
		{
			buffer_count--;
		}
		if(state.graphics.draw_buffer != tz::nullhand)
		{
			buffer_count--;
		}

		// now we can actually create the descriptor layout.
		vk2::DescriptorLayoutBuilder builder;
		builder.set_device(tz::gl::get_device().vk_get_logical_device());
		for(std::size_t i = 0; i < buffer_count; i++)
		{
			builder.with_binding
			({
				.type = vk2::DescriptorType::StorageBuffer,
				.count = 1,
				.flags =
				{
					vk2::DescriptorFlag::UpdateAfterBind,
					vk2::DescriptorFlag::UpdateUnusedWhilePending
				}
			});
		}
		if(image_count > 0)
		{
			builder.with_binding
			({
				.type = vk2::DescriptorType::ImageWithSampler,
				.count = static_cast<std::uint32_t>(image_count),
				.flags = 
				{
					vk2::DescriptorFlag::PartiallyBound,
					vk2::DescriptorFlag::UpdateAfterBind,
					vk2::DescriptorFlag::UpdateUnusedWhilePending
				}
			});
		}
		this->layout = builder.build();
	}

//--------------------------------------------------------------------------------------------------

	renderer_vulkan2::renderer_vulkan2(const tz::gl::renderer_info& rinfo):
	renderer_descriptor_manager(rinfo),
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
