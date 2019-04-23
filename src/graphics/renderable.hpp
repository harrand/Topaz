//
// Created by Harrand on 14/04/2019.
//

#ifndef TOPAZ_RENDERABLE_HPP
#define TOPAZ_RENDERABLE_HPP
#include "graphics/render_pass.hpp"


class Renderable
{
public:
	Renderable();
	virtual std::optional<AABB> get_boundary() const = 0;
	virtual void render(RenderPass render_pass) const = 0;
private:
};


#endif //TOPAZ_RENDERABLE_HPP
