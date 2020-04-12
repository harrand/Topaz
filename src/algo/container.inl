namespace tz::algo
{
    template<typename StandardContainer>
    constexpr std::size_t sizeof_element([[maybe_unused]] const StandardContainer& container)
    {
        return sizeof_element<StandardContainer>();
    }

    template<typename StandardContainer>
    constexpr std::size_t sizeof_element()
    {
        return sizeof(typename StandardContainer::value_type);
    }
}