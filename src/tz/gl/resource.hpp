#ifndef TOPAZ_GL2_RESOURCE_HPP
#define TOPAZ_GL2_RESOURCE_HPP
#include "tz/gl/api/resource.hpp"
#include "tz/gl/declare/image_format.hpp"
#include "tz/core/types.hpp"
#include "hdk/data/vector.hpp"
#include <ranges>
#include <optional>

namespace tz::gl
{
	class Resource : public iresource
	{
	public:
		virtual ~Resource() = default;
		// iresource
		virtual resource_type get_type() const final;
		virtual resource_access get_access() const final;
		virtual const resource_flags& get_flags() const final;
		virtual std::span<const std::byte> data() const final;
		virtual std::span<std::byte> data() final;
		virtual void dbgui() override;

		void resize_data(std::size_t new_size);
	protected:
		Resource(resource_access access, std::vector<std::byte> resource_data, std::size_t initial_alignment_offset, resource_type type, resource_flags flags = {});
		virtual void set_mapped_data(std::span<std::byte> mapped_resource_data) override;
	private:
		resource_access access;
		std::vector<std::byte> resource_data;
		std::optional<std::span<std::byte>> mapped_resource_data;
		std::size_t initial_alignment_offset;
		resource_type type;
		resource_flags flags;
	};

	struct BufferInfo
	{
		resource_access access = resource_access::static_fixed;
		resource_flags flags = {};
	};

	/**
	 * @ingroup tz_gl2_res
	 * Represents a fixed-size, static Buffer to be used by a renderer or Processor.
	 */
	class BufferResource : public Resource
	{
	public:
		virtual ~BufferResource() = default;
		/**
		 * Create a BufferResource where the underlying data is a single object.
		 * @note You should be able to optionally pass in braced-initializer-list expressions in for the data, so long as the types of the elements are easily deduceable.
		 * @tparam T Object type. It must be trivially_copyable.
		 * @param data Object value to store within the underlying data.
		 * @param info Buffer info, see @ref BufferInfo for details.
		 * @return BufferResource containing a copy of the provided object.
		 */
		template<tz::trivially_copyable T>
		static BufferResource from_one(const T& data, BufferInfo info = {});

		template<tz::trivially_copyable T>
		static BufferResource from_many(std::initializer_list<T> ts, BufferInfo info = {})
		{
			return from_many(std::span<const T>(ts), info);
		}
		/**
		 * Create a BufferResource where the underlying data is an array of objects.
		 * @tparam R Type satisfying std::contiguous_range.
		 * @param data A range of elements of some type.
		 * @param info Buffer info. See @ref BufferInfo for details.
		 * @return BufferResource containing a copy of the provided array.
		 */
		template<std::ranges::contiguous_range R>
		static BufferResource from_many(R&& data, BufferInfo info = {});
		/**
		 * Create a null BufferResource. It is not practically useful, aside from as a placeholder.
		 *
		 * Null BufferResources are guaranteed to have size() == 1, not zero, but its contents and size are implementation-defined. It is also guaranteed to be static_fixed and have no flags.
		 */
		static BufferResource null()
		{
			return from_one(std::byte{255}, {});
		}

		bool is_null() const;
		virtual std::unique_ptr<iresource> unique_clone() const final;
		virtual void dbgui() final;
	private:
		BufferResource(resource_access access, std::vector<std::byte> resource_data, std::size_t initial_alignment_offset, resource_flags flags);
	};


	/**
	 * Represents creation flags for an image.
	 */
	struct ImageInfo
	{
		/// Image format.
		image_format format;
		/// Image dimensions, in pixels.
		hdk::vec2ui dimensions;
		/// Access specifier. By default this is static fixed.
		resource_access access = resource_access::static_fixed;
		/// Flags specifying any special usages for the image. By default there are no flags.
		resource_flags flags = {};
	};

	/**
	 * @ingroup tz_gl2_res
	 * Represents a fixed-size, static Image to be used by a renderer or Processor.
	 */
	class ImageResource : public Resource
	{
	public:
		virtual ~ImageResource() = default;
		/**
		 * Create an ImageResource where the image-data is uninitialised. See @ref ImageInfo for details.
		 * @return ImageResource containing uninitialised image-data.
		 */
		static ImageResource from_uninitialised(ImageInfo info = {});

		template<tz::trivially_copyable T>
		static ImageResource from_memory(std::initializer_list<T> ts, ImageInfo info = {})
		{
			return from_memory(std::span<const T>(ts), info);
		}
		/**
		 * Create an ImageResource using values existing in memory.
		 * @note You should be able to optionally pass in braced-initializer-list expressions in for the data, so long as the types of the elements are easily deduceable.
		 * @param data Range containing a block of memory representing the image data. The length of the block should exactly match that of the image's size in bytes, or the behaviour is undefined.
		 */
		static ImageResource from_memory(std::ranges::contiguous_range auto data, ImageInfo info = {});
		/**
		 * Create a null ImageResource.
		 *
		 * The format, dimensions and image values are all implementation-defined, but the access is guaranteed to be static_fixed.
		 */
		static ImageResource null()
		{
			return from_memory
			(
			 	{
					// Missingno purple
					(unsigned char)0b1111'1111,
					(unsigned char)0b0000'0000,
					(unsigned char)0b1111'1111,
					(unsigned char)0b1111'1111,
					// Black
					(unsigned char)0b0000'0000,
					(unsigned char)0b0000'0000,
					(unsigned char)0b0000'0000,
					(unsigned char)0b1111'1111,
					// Black
					(unsigned char)0b0000'0000,
					(unsigned char)0b0000'0000,
					(unsigned char)0b0000'0000,
					(unsigned char)0b1111'1111,
					// Missingno purple
					(unsigned char)0b1111'1111,
					(unsigned char)0b0000'0000,
					(unsigned char)0b1111'1111,
					(unsigned char)0b1111'1111
				},
				{.format = image_format::RGBA32, .dimensions = {2u, 2u}}
			);
		}
	
		bool is_null() const;

		virtual std::unique_ptr<iresource> unique_clone() const final;
		virtual void dbgui() final;
		image_format get_format() const;
		hdk::vec2ui get_dimensions() const;
		void set_dimensions(hdk::vec2ui dims);
	private:
		ImageResource(resource_access access, std::vector<std::byte> resource_data, std::size_t initial_alignment_offset, image_format format, hdk::vec2ui dimensions, resource_flags flags);
		image_format format;
		hdk::vec2ui dimensions;
	};
}
#include "tz/gl/resource.inl"

#endif // TOPAZ_GL2_RESOURCE_HPP
