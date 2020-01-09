#include "geo/vector.hpp"

namespace tz::geo
{
    Vec4::Vec4(float x, float y, float z, float w): handle(_mm_setr_ps(x, y, z, w)){}

    float Vec4::get_x() const
    {
        return this->get()[0];
    }

    float Vec4::get_y() const
    {
        return this->get()[1];
    }

    float Vec4::get_z() const
    {
        return this->get()[2];
    }

    float Vec4::get_w() const
    {
        return this->get()[3];
    }

    Vec4Data Vec4::get() const
    {
        Vec4Data floats;
        _mm_storeu_ps(floats.data(), this->handle);
        return floats;
    }

    void Vec4::set(Vec4Data vals)
    {
        this->handle = _mm_loadr_ps(vals.data());
    }

    Vec4 Vec4::operator+(const Vec4& rhs) const
    {
        return {_mm_add_ps(this->handle, rhs.handle)};
    }

    Vec4 Vec4::operator-(const Vec4& rhs) const
    {
        return {_mm_sub_ps(this->handle, rhs.handle)};
    }

    Vec4& Vec4::operator+=(const Vec4& rhs)
    {
        *this = *this + rhs;
        return *this;
    }

    Vec4& Vec4::operator-=(const Vec4& rhs)
    {
        *this = *this - rhs;
        return *this;
    }

    Vec4::Vec4(sse::FloatHandle direct_handle): handle(direct_handle){}
}