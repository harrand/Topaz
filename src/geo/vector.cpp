#include "geo/vector.hpp"
#include <cmath>

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

    float Vec4::length() const
    {
        return std::sqrt(this->dot(*this));
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

    float Vec4::dot(const Vec4& rhs) const
    {
        sse::FloatHandle ret = _mm_dp_ps(this->handle, rhs.handle, 0b11110001);
        return _mm_cvtss_f32(ret);
    }

    Vec4::Vec4(sse::FloatHandle direct_handle): handle(direct_handle){}
}