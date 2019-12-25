//
// Created by Harrand on 25/12/2019.
//

#include "gl/draw_command.hpp"

namespace tz::gl
{
    void IDrawCommand::operator()() const
    {
        this->invoke();
    }
}