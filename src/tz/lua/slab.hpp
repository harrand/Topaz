#ifndef TOPAZ_LUA_SLAB_HPP
#define TOPAZ_LUA_SLAB_HPP
#include <string>
#include <any>
#include <vector>
#include <span>

namespace tz::lua
{
	struct lua_operation
	{
		unsigned int context_id;
		unsigned int subject_id;
		std::any value = {};
	};

	class empty_slab_processor;

	class i_slab_processor
	{
	public:
		virtual ~i_slab_processor() = default;
		virtual lua_operation merge(lua_operation lhs, lua_operation rhs) const = 0;
		virtual void process_operation(lua_operation op) = 0;
		virtual std::string describe(const lua_operation& op) const
		{
			return {};
		}

		constexpr virtual bool does_anything() const
		{
			return !std::is_same_v<decltype(*this), empty_slab_processor>;
		}
	};

	class empty_slab_processor : public i_slab_processor
	{
		virtual lua_operation merge(lua_operation lhs, lua_operation rhs) const override final
		{
			return lhs;
		}

		virtual void process_operation(lua_operation op) override final
		{
			
		}
	};

	class slab
	{
	public:
		slab() = default;
		void clear();
		std::size_t size() const;
		std::span<const lua_operation> data() const;
	private:
		std::vector<lua_operation> operations = {};
	};

	slab& get_slab();
	i_slab_processor& get_slab_processor();
	void install_slab_processor(i_slab_processor& proc);
	void process_slab();
}

#endif // TOPAZ_LUA_SLAB_HPP
