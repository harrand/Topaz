#ifndef TOPAZ_GL2_RESOURCE_HPP
#define TOPAZ_GL2_RESOURCE_HPP
#include "gl/2/api/resource.hpp"
#include "gl/2/declare/image_format.hpp"
#include "core/types.hpp"
#include "core/vector.hpp"

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
		Resource(std::vector<std::byte> resource_data, std::size_t initial_alignment_offset, ResourceType type);
	private:
		std::vector<std::byte> resource_data;
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
		static BufferResource from_one(const T& data);

		/**
		 * Create a BufferResource where the underlying data is an array of objects.
		 * @tparam T Array element type. It must be TriviallyCopyable.
		 * @param data View into an array. The data will be copied from this span into the underlying buffer data.
		 * @return BufferResource containing a copy of the provided array.
		 */
		template<tz::TriviallyCopyable T>
		static BufferResource from_many(std::span<const T> data);
		virtual std::unique_ptr<IResource> unique_clone() const final;
	private:
		BufferResource(std::vector<std::byte> resource_data, std::size_t initial_alignment_offset);
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
		static ImageResource from_uninitialised(ImageFormat format, tz::Vec2ui dimensions);
		virtual std::unique_ptr<IResource> unique_clone() const final;
		ImageFormat get_format() const;
		tz::Vec2ui get_dimensions() const;
	private:
		ImageResource(std::vector<std::byte> resource_data, std::size_t initial_alignment_offset, ImageFormat format, tz::Vec2ui dimensions);
		ImageFormat format;
		tz::Vec2ui dimensions;
	};
}
#include "gl/2/resource.inl"

#endif // TOPAZ_GL2_RESOURCE_HPP
