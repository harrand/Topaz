#ifndef TOPAZ_GL2_RESOURCE_HPP
#define TOPAZ_GL2_RESOURCE_HPP
#include "gl/api/resource.hpp"
#include "gl/declare/image_format.hpp"
#include "core/types.hpp"
#include "core/vector.hpp"
#include <ranges>
#include <optional>

namespace tz::gl2
{
	class Resource : public IResource
	{
	public:
		virtual ~Resource() = default;
		// IResource
		virtual ResourceType get_type() const final;
		virtual ResourceAccess get_access() const final;
		virtual std::span<const std::byte> data() const final;
		virtual std::span<std::byte> data() final;
	protected:
		Resource(ResourceAccess access, std::vector<std::byte> resource_data, std::size_t initial_alignment_offset, ResourceType type);
		virtual void set_mapped_data(std::span<std::byte> mapped_resource_data) override;
	private:
		ResourceAccess access;
		std::vector<std::byte> resource_data;
		std::optional<std::span<std::byte>> mapped_resource_data;
		std::size_t initial_alignment_offset;
		ResourceType type;
	};

	/**
	 * @ingroup tz_gl2_res
	 * Represents a fixed-size, static Buffer to be used by a Renderer or Processor.
	 */
	class BufferResource : public Resource
	{
	public:
		virtual ~BufferResource() = default;
		/**
		 * Create a BufferResource where the underlying data is a single object.
		 * @tparam T Object type. It must be TriviallyCopyable.
		 * @param data Object value to store within the underlying data.
		 * @return BufferResource containing a copy of the provided object.
		 */
		template<tz::TriviallyCopyable T>
		static BufferResource from_one(const T& data, ResourceAccess access = ResourceAccess::StaticFixed);

		template<tz::TriviallyCopyable T>
		static BufferResource from_many(std::initializer_list<T> ts, ResourceAccess access = ResourceAccess::StaticFixed)
		{
			return from_many(std::span<const T>(ts), access);
		}
		/**
		 * Create a BufferResource where the underlying data is an array of objects.
		 * @tparam T Array element type. It must be TriviallyCopyable.
		 * @param data View into an array. The data will be copied from this span into the underlying buffer data.
		 * @return BufferResource containing a copy of the provided array.
		 */
		template<std::ranges::contiguous_range R>
		static BufferResource from_many(R&& data, ResourceAccess access = ResourceAccess::StaticFixed);
		virtual std::unique_ptr<IResource> unique_clone() const final;
	private:
		BufferResource(ResourceAccess access, std::vector<std::byte> resource_data, std::size_t initial_alignment_offset);
	};

	struct DynamicBufferResource
	{
		template<tz::TriviallyCopyable T>
		static BufferResource from_one(const T& data)
		{
			return BufferResource::from_one({data}, ResourceAccess::DynamicFixed);
		}
		template<typename T>
		static BufferResource from_many(std::initializer_list<T> data)
		{
			return from_many(std::span<const T>(data));
		}
		static BufferResource from_many(std::ranges::contiguous_range auto data)
		{
			return BufferResource::from_many(data, ResourceAccess::DynamicFixed);
		}
	};

	/**
	 * @ingroup tz_gl2_res
	 * Represents a fixed-size, static Image to be used by a Renderer or Processor.
	 */
	class ImageResource : public Resource
	{
	public:
		virtual ~ImageResource() = default;
		/**
		 * Create an ImageResource where the image-data is uninitialised.
		 * @param format ImageFormat of the data. It must not be ImageFormat::Undefined.
		 * @param dimensions {width, height} of the image, in pixels.
		 * @return ImageResource containing uninitialised image-data of the given format and dimensions.
		 */
		static ImageResource from_uninitialised(ImageFormat format, tz::Vec2ui dimensions, ResourceAccess access = ResourceAccess::StaticFixed);

		template<tz::TriviallyCopyable T>
		static ImageResource from_memory(ImageFormat fmt, tz::Vec2ui dimensions, std::initializer_list<T> ts, ResourceAccess access = ResourceAccess::StaticFixed)
		{
			return from_memory(fmt, dimensions, std::span<const T>(ts), access);
		}
		/**
		 * Create an ImageResource using values existing in memory.
		 * @param format ImageFormat of the data. It must not be ImageFormat::Undefined.
		 * @param dimensions {width, height} of the image, in pixels.
		 * @param byte_data Array of bytes, length equal to `tz::gl2::pixel_size_bytes(format) * dimensions[0] * dimensions[1]`
		 * @return ImageResource containing an image using the provided data.
		 * @pre `byte_data` exactly matches the number of bytes expected in the explanation above. Otherwise, the behaviour is undefined.
		 */
		static ImageResource from_memory(ImageFormat format, tz::Vec2ui dimensions, std::ranges::contiguous_range auto data, ResourceAccess access = ResourceAccess::StaticFixed);
		virtual std::unique_ptr<IResource> unique_clone() const final;
		ImageFormat get_format() const;
		tz::Vec2ui get_dimensions() const;
	private:
		ImageResource(ResourceAccess access, std::vector<std::byte> resource_data, std::size_t initial_alignment_offset, ImageFormat format, tz::Vec2ui dimensions);
		ImageFormat format;
		tz::Vec2ui dimensions;
	};
}
#include "gl/resource.inl"

#endif // TOPAZ_GL2_RESOURCE_HPP
