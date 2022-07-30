#ifndef TOPAZ_CORE_MEMORY_HPP
#define TOPAZ_CORE_MEMORY_HPP
#include <memory>
#include <variant>

namespace tz
{
	/**
	 * @ingroup tz_core
	 * Represents an arbitrary, non-owning block of memory.
	 */
	struct Blk
	{
		/// Pointer to the start of the memory block.
		void* ptr;
		/// Size of the block, in bytes.
		std::size_t size;

		bool operator==(const Blk& rhs) const = default;
	};
	constexpr Blk nullblk{.ptr = nullptr, .size = 0};

	template<typename T>
	class MaybeOwnedPtr
	{
	public:
		MaybeOwnedPtr(std::nullptr_t);
		MaybeOwnedPtr(T* ptr);
		MaybeOwnedPtr(std::unique_ptr<T> owned);
		template<typename P>
		MaybeOwnedPtr(MaybeOwnedPtr<P>&& move) requires std::derived_from<P, T>;

		MaybeOwnedPtr(const MaybeOwnedPtr<T>& copy) = default;
		MaybeOwnedPtr(MaybeOwnedPtr<T>&& move) = default;
		MaybeOwnedPtr& operator=(const MaybeOwnedPtr<T>& rhs) = default;
		MaybeOwnedPtr& operator=(MaybeOwnedPtr<T>&& rhs) = default;
		MaybeOwnedPtr& operator=(std::nullptr_t);
		MaybeOwnedPtr& operator=(T* ptr);
		MaybeOwnedPtr& operator=(std::unique_ptr<T> ptr);

		T* get();
		const T* get() const;
		bool owning() const;
		void reset();
		T* release();
		void set_owning(bool should_own);

		template<typename P>
		P* as() requires std::derived_from<P, T>
		{
			return static_cast<P*>(this->get());
		}

		template<typename P>
		const P* as() const requires std::derived_from<P, T>
		{
			return static_cast<const P*>(this->get());
		}
		
		T* operator->();
		const T* operator->() const;
		bool operator==(const T* t) const;
	private:
		using Variant = std::variant<T*, std::unique_ptr<T>>;
		Variant ptr;
	};

	template<typename T, typename... Args>
	MaybeOwnedPtr<T> make_owned(Args&&... args);
}
#include "tz/core/memory.inl"

#endif // TOPAZ_CORE_MEMORY_HPP
