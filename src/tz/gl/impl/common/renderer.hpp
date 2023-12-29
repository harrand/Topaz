#ifndef TOPAZ_GL2_IMPL_FRONTEND_COMMON_RENDERER_HPP
#define TOPAZ_GL2_IMPL_FRONTEND_COMMON_RENDERER_HPP
#include "tz/core/data/handle.hpp"
#include "tz/core/debug.hpp"
#include "tz/core/memory/maybe_owned_ptr.hpp"
#include <span>
#include <vector>

namespace tz::gl
{
	template<class Asset>
	class AssetStorageCommon
	{
	public:
		using AssetHandle = tz::handle<Asset>;
		AssetStorageCommon(std::span<const Asset* const> assets = {}):
		asset_storage()
		{
			for(const Asset* asset : assets)
			{
				if(asset == nullptr)
				{
					this->asset_storage.push_back(nullptr);
				}
				else
				{
					this->asset_storage.push_back(asset->unique_clone());
				}
			}
		}

		unsigned int count() const
		{
			return this->asset_storage.size();
		}
		
		const Asset* get(AssetHandle handle) const
		{
			if(handle == tz::nullhand) return nullptr;
			std::size_t handle_val = static_cast<std::size_t>(static_cast<tz::hanval>(handle));
			return this->asset_storage[handle_val].get();
		}

		Asset* get(AssetHandle handle)
		{
			if(handle == tz::nullhand) return nullptr;
			std::size_t handle_val = static_cast<std::size_t>(static_cast<tz::hanval>(handle));
			return this->asset_storage[handle_val].get();
		}

		void set(AssetHandle handle, Asset* value)
		{
			std::size_t handle_val = static_cast<std::size_t>(static_cast<tz::hanval>(handle));
			tz::assert(!this->asset_storage[handle_val].owning(), "AssetStorageCommon: Try to set specific asset value, but the asset at that handle is not a reference (it is owned by us)");
			this->asset_storage[handle_val] = value;
		}
	private:
		std::vector<maybe_owned_ptr<Asset>> asset_storage;
	};

}

#endif // TOPAZ_GL2_IMPL_FRONTEND_COMMON_RENDERER_HPP
