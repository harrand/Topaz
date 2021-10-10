#ifndef TOPAZ_GL_API_RESOURCE_HPP
#define TOPAZ_GL_API_RESOURCE_HPP
#include "core/interfaces/cloneable.hpp"
#include "core/handle.hpp"
#include "gl/declare/resource.hpp"
#include "gl/declare/renderer.hpp"
#include <span>

namespace tz::gl
{
	/**
	 * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
	 * A collection of low-level renderer-agnostic graphical interfaces.
	 * @{
	 */
	/**
	 * @brief Renderers can be associated with zero or more resources. Resources are extra components which might be required rendering, such as textures or uniform buffers.
	 * @details Resources can vary wildly in their nature depending on what sort of rendering you'd like to do. If you don't provide any resources to a renderer, it will simply draw its input to its output.
	 * 
	 * However, if you have extra data which you would like to use in shaders, then you can create resources and pass them to the @ref IRendererBuilder so it can be used by the renderer. The most common use-case is for adding model, view and projection data via uniform buffers.
	 */
	class IResource : public tz::IUniqueCloneable<IResource>
	{
	public:
		virtual ~IResource() = default;
		/**
		 * @brief Obtain the type represented by this resource.
		 * 
		 * @return Type representing resource data for the @ref IRenderer to understand.
		 */
		virtual constexpr ResourceType get_type() const = 0;
		/**
		 * @brief Retrieve the data access specifier for this resource type.
		 * @note Resource derived from @ref IResource are `StaticFixed` by default, but this can be overriden. Inputs derived from @ref IDynamicResource are always `DynamicFixed` and this cannot be overridden.
		 * 
		 * @return Access specifier for the data relative to the @ref IRenderer.
		 */
		virtual constexpr RendererInputDataAccess data_access() const {return RendererInputDataAccess::StaticFixed;}
		/**
		 * @brief Retrieve the resource data as bytes. The data within the span is immutable.
		 * @note See @ref IDynamicResource::get_resource_bytes_dynamic() for the option of mutable resource data.
		 * 
		 * @return std::span<const std::byte> displaying the byte-representation of the resource data.
		 */
		virtual std::span<const std::byte> get_resource_bytes() const = 0;
	};

	/// Opaque handle which can be provided to @ref IRenderer::get_resource() to retrieve a resource.
	using ResourceHandle = tz::Handle<IResource>;

	/**
	 * @brief Identical to @ref IResource, but `IResourceCopyable::unique_clone()` need not be implemented.
	 * @pre Derived must be copy-constructible. Otherwise, the program is ill-formed.
	 * 
	 * @tparam Derived Renderer resource type. It must be copy-constructible.
	 */
	template<typename Derived>
	class IResourceCopyable : public IResource
	{
	public:
		/// Invokes `Derived::Derived(const Derived&)`
		[[nodiscard]] virtual std::unique_ptr<IResource> unique_clone() const final
		{
			static_assert(std::is_copy_constructible_v<Derived>, "IResourceCopyable<T>: T must be copyable. Derive from IResource and implement unique_clone if not copyable.");
			return std::make_unique<Derived>(static_cast<const Derived&>(*this));
		}
	};

	/**
	 * @brief Similar to @ref IResource, but the resource data can be changed at any point, even while used by a @ref IRenderer.
	 */
	class IDynamicResource : public IResource
	{
	public:
		/**
		 * @brief Retrieve the data access specifier for this render resource type.
		 * @note Inputs derived from @ref IResource are `StaticFixed` by default. Inputs derived from @ref IDynamicResource are always `DynamicFixed`.
		 * 
		 * @return constexpr RendererInputDataAccess 
		 */
		virtual constexpr RendererInputDataAccess data_access() const final{return RendererInputDataAccess::DynamicFixed;}
		/**
		 * @brief Retrieve the resource data as bytes. The data within the span is mutable.
		 * @note Aside from mutability, this is functionally identical to @ref IResource::get_resource_bytes().
		 * @note Dynamic resource data can be edited on-the-fly -- It is valid to edit the data while the resource is in-use by a renderer, in which case the updated values are guaranteed to be visible in the next render invocation.
		 * 
		 * @return std::span<std::byte> displaying the byte-representation of the resource data.
		 */
		virtual std::span<std::byte> get_resource_bytes_dynamic() = 0;

		#if TZ_VULKAN
			friend class RendererBufferManagerVulkan;
			friend class ProcessorResourceManagerVulkan;
		#elif TZ_OGL
			friend class RendererOGL;
			friend class ProcessorOGL;
		#endif
	private:
		virtual void set_resource_data(std::byte* resource_data) = 0;
	};

	/**
	 * @brief Identical to IDynamicResource, but `IDynamicResourceCopyable::unique_clone()` need not be implemented.
	 * @pre Derived must be copy-constructible. Otherwise, the program is ill-formed.
	 * 
	 * @tparam Derived Renderer resource tpye. It must be copy-constructible.
	 */
	template<typename Derived>
	class IDynamicResourceCopyable : public IDynamicResource
	{
	public:
		/// Invokes `Derived::Derived(const Derived&)`
		[[nodiscard]] virtual std::unique_ptr<IResource> unique_clone() const final
		{
			static_assert(std::is_copy_constructible_v<Derived>, "IDynamicResourceCopyable<T>: T must be copyable. Derive from IDynamicResource and implement unique_clone if not copyable.");
			return std::make_unique<Derived>(static_cast<const Derived&>(*this));
		}
	};

	/**
	 * @}
	 */
}

#endif // TOPAZ_GL_API_RESOURCE_HPP