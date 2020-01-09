namespace tz::geo
{
    namespace sse
    {
        template<std::size_t Quantity>
        __m128* handle_array(SSEValidArray<float, Quantity>& aligned_data)
        {
            return reinterpret_cast<__m128*>(aligned_data);
        }
    }

    template<typename T, std::size_t Quantity>
    constexpr VecData<T, Quantity>::VecData(std::array<T, Quantity> array): arr(array){}

    template<typename T, std::size_t Quantity>
    constexpr const T* VecData<T, Quantity>::data() const
    {
        return this->arr.data();
    }

    template<typename T, std::size_t Quantity>
    constexpr T* VecData<T, Quantity>::data()
    {
        return this->arr.data();
    }

    template<typename T, std::size_t Quantity>
    constexpr const T& VecData<T, Quantity>::operator[](std::size_t idx) const
    {
        return this->arr[idx];
    }

    template<typename T, std::size_t Quantity>
    constexpr T& VecData<T, Quantity>::operator[](std::size_t idx)
    {
        return this->arr[idx];
    }

    template<typename T>
    constexpr VecData<T, 1>::VecData(T value): val(value){}

    template<typename T>
    constexpr const T* VecData<T, 1>::data() const
    {
        return &this->val;
    }

    template<typename T>
    constexpr T* VecData<T, 1>::data()
    {
        return &this->val;
    }

    template<typename T>
    constexpr VecData<T, 1>::operator T() const
    {
        return this->val;
    }

    template<typename T>
    constexpr VecData<T, 1>::operator T()
    {
        return this->val;
    }
}