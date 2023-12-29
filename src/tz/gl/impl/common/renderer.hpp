#ifndef TOPAZ_GL2_IMPL_FRONTEND_COMMON_RENDERER_HPP
#define TOPAZ_GL2_IMPL_FRONTEND_COMMON_RENDERER_HPP
#include "tz/core/data/handle.hpp"
#include "tz/core/debug.hpp"
#include "tz/core/data/maybe_owned_list.hpp"
#include <span>
#include <vector>

namespace tz::gl
{
	template<class Asset>
	using AssetStorageCommon = tz::maybe_owned_list<Asset>;
}

#endif // TOPAZ_GL2_IMPL_FRONTEND_COMMON_RENDERER_HPP
