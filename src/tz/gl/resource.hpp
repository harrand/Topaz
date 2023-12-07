#ifndef TOPAZ_GL2_RESOURCE_HPP
#define TOPAZ_GL2_RESOURCE_HPP
#include "tz/gl/api/resource.hpp"
#include "tz/gl/declare/image_format.hpp"
#include "tz/core/types.hpp"
#include "tz/core/data/vector.hpp"
#include "tz/io/image.hpp"
#include <ranges>
#include <optional>

namespace tz::gl
{
	class resource : public iresource
	{
	public:
		virtual ~resource() = default;
		// iresource
		virtual resource_type get_type() const final;
		virtual resource_access get_access() const final;
		virtual const resource_flags& get_flags() const final;
		virtual std::span<const std::byte> data() const final;
		virtual std::span<std::byte> data() final;
		virtual void dbgui() override;

		void resize_data(std::size_t new_size);
	protected:
		resource(resource_access access, std::vector<std::byte> resource_data, std::size_t initial_alignment_offset, resource_type type, resource_flags flags = {});
		virtual void set_mapped_data(std::span<std::byte> mapped_resource_data) override;
		resource_access access;
		std::vector<std::byte> resource_data;
	private:
		std::optional<std::span<std::byte>> mapped_resource_data;
		std::size_t initial_alignment_offset;
		resource_type type;
		resource_flags flags;
	};

	struct buffer_info
	{
		resource_access access = resource_access::static_access;
		resource_flags flags = {};
	};

	/**
	 * @ingroup tz_gl2_res
	 * Represents a fixed-size, static Buffer to be used by a renderer or Processor.
	 */
	class buffer_resource : public resource
	{
	public:
		virtual ~buffer_resource() = default;
		/**
		 * Create a buffer_resource where the underlying data is a single object.
		 * @note You should be able to optionally pass in braced-initializer-list expressions in for the data, so long as the types of the elements are easily deduceable.
		 * @tparam T Object type. It must be trivially_copyable.
		 * @param data Object value to store within the underlying data.
		 * @param info Buffer info, see @ref buffer_info for details.
		 * @return buffer_resource containing a copy of the provided object.
		 */
		template<tz::trivially_copyable T>
		static buffer_resource from_one(const T& data, buffer_info info = {});

		template<tz::trivially_copyable T>
		static buffer_resource from_many(std::initializer_list<T> ts, buffer_info info = {})
		{
			return from_many(std::span<const T>(ts), info);
		}
		/**
		 * Create a buffer_resource where the underlying data is an array of objects.
		 * @tparam R Type satisfying std::contiguous_range.
		 * @param data A range of elements of some type.
		 * @param info Buffer info. See @ref buffer_info for details.
		 * @return buffer_resource containing a copy of the provided array.
		 */
		template<std::ranges::contiguous_range R>
		static buffer_resource from_many(R&& data, buffer_info info = {});
		/**
		 * Create a null buffer_resource. It is not practically useful, aside from as a placeholder.
		 *
		 * Null BufferResources are guaranteed to have size() == 1, not zero, but its contents and size are implementation-defined. It is also guaranteed to be static_fixed and have no flags.
		 */
		static buffer_resource null()
		{
			return from_one(std::byte{255}, {});
		}

		bool is_null() const;
		virtual std::unique_ptr<iresource> unique_clone() const final;
		virtual void dbgui() final;
	private:
		buffer_resource(resource_access access, std::vector<std::byte> resource_data, std::size_t initial_alignment_offset, resource_flags flags);
	};


	/**
	 * Represents creation flags for an image.
	 */
	struct image_info
	{
		/// Image format.
		image_format format;
		/// Image dimensions, in pixels.
		tz::vec2ui dimensions;
		/// Access specifier. By default this is static fixed.
		resource_access access = resource_access::static_access;
		/// Flags specifying any special usages for the image. By default there are no flags.
		resource_flags flags = {};
	};

	/**
	 * @ingroup tz_gl2_res
	 * Represents a fixed-size, static Image to be used by a renderer or Processor.
	 */
	class image_resource : public resource
	{
	public:
		virtual ~image_resource() = default;
		/**
		 * Create an image_resource where the image-data is uninitialised. See @ref image_info for details.
		 * @return image_resource containing uninitialised image-data.
		 */
		static image_resource from_uninitialised(image_info info = {});

		template<tz::trivially_copyable T>
		static image_resource from_memory(std::initializer_list<T> ts, image_info info = {})
		{
			return from_memory(std::span<const T>(ts), info);
		}
		/**
		 * Create an image_resource using values existing in memory.
		 * @note You should be able to optionally pass in braced-initializer-list expressions in for the data, so long as the types of the elements are easily deduceable.
		 * @param data Range containing a block of memory representing the image data. The length of the block should exactly match that of the image's size in bytes, or the behaviour is undefined.
		 */
		static image_resource from_memory(std::ranges::contiguous_range auto data, image_info info = {});

		static image_resource from_struct(tz::io::image img, image_info info = {});
		/**
		 * Create a null image_resource.
		 *
		 * The format, dimensions and image values are all implementation-defined, but the access is guaranteed to be static_fixed.
		 */
		static image_resource null()
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
		tz::vec2ui get_dimensions() const;
		void set_dimensions(tz::vec2ui dims);
	private:
		image_resource(resource_access access, std::vector<std::byte> resource_data, std::size_t initial_alignment_offset, image_format format, tz::vec2ui dimensions, resource_flags flags);
		image_format format;
		tz::vec2ui dimensions;
	};
}
#include "tz/gl/resource.inl"

#endif // TOPAZ_GL2_RESOURCE_HPP
