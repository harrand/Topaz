#ifndef TOPAZ_GEOMETRY_VECTOR_HPP
#define TOPAZ_GEOMETRY_VECTOR_HPP
#include "memory/align.hpp"
#include <immintrin.h>

namespace tz::geo
{
    namespace sse
    {
        template<typename T, std::size_t Quantity>
        using SSEValidArray = tz::mem::AlignedTypeArray<T, 16, Quantity>;

        using FloatHandle = __m128;

        /**
         * Risk a strict-aliasing violation to perform a conversion from an aligned array of floats to an SSE handle at minimal cost.
         */
        template<std::size_t Quantity>
        FloatHandle* handle_array(SSEValidArray<float, Quantity>& aligned_data);
    }

    template<typename T, std::size_t Quantity>
    class VecData
    {
    public:
        constexpr VecData(std::array<T, Quantity> array = {});
        constexpr const T* data() const;
        constexpr T* data();
        constexpr const T& operator[](std::size_t idx) const;
        constexpr T& operator[](std::size_t idx);
    private:
        std::array<T, Quantity> arr;
    };

    template<typename T>
    class VecData<T, 1>
    {
    public:
        constexpr VecData(T value);
        constexpr const T* data() const;
        constexpr T* data();
        constexpr operator T() const;
        constexpr operator T();
    private:
        T val;
    };

    using Vec4Data = VecData<float, 4>;

    class Vec4
    {
    public:
        Vec4(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 0.0f);
        float get_x() const;
        float get_y() const;
        float get_z() const;
        float get_w() const;
        void set_x(float x) const;
        void set_y(float y) const;
        void set_z(float z) const;
        void set_w(float w) const;
        Vec4Data get() const;
        float length() const;
        void set(Vec4Data vals);
        Vec4 operator+(const Vec4& rhs) const;
        Vec4 operator-(const Vec4& rhs) const;
        Vec4& operator+=(const Vec4& rhs);
        Vec4& operator-=(const Vec4& rhs);
        float dot(const Vec4& rhs) const;
    private:
        Vec4(sse::FloatHandle direct_handle);

        sse::FloatHandle handle;
    };
}

#include "geo/vector.inl"
#endif // TOPAZ_GEOMETRY_VECTOR_HPP