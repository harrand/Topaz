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
}