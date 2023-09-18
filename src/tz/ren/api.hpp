#ifndef TOPAZ_REN_API_HPP
#define TOPAZ_REN_API_HPP

namespace tz::ren
{
	/**
	 * @ingroup tz_ren
	 * Interface for a high-level `tz::ren` renderer class.
	 */
	class ihigh_level_renderer
	{
	public:
		/**
		 * Append this renderer to the end of the render-graph, without specifying any dependencies.
		 */
		virtual void append_to_render_graph() = 0;
	};
}

#endif // TOPAZ_REN_API_HPP