#include <cstring>

namespace tz::gl
{
    template<typename T>
    BasicResourceWriter<T>::BasicResourceWriter(tz::mem::Block data): data(data), write_count(0)
    {

    }

    template<typename T>
    bool BasicResourceWriter<T>::write(T element)
    {
        std::size_t offset_bytes = this->write_count * sizeof(T);
        if(offset_bytes >= this->data.size())
        {
            // Ain't enough space.
            return false;
        }
        // Let's write. If POD we will respect lifetime rules.
        char* loc = reinterpret_cast<char*>(this->data.begin) + offset_bytes;
        if constexpr(std::is_pod_v<T>)
        {
            // POD. Just memcpy.
            std::memcpy(loc, &element, sizeof(T));
        }
        else
        {
            // Not POD, use in-place new.
            new (loc) T{element};
        }
        this->write_count++;
        return true;
    }

    template<typename T>
    void BasicResourceWriter<T>::reset()
    {
        this->write_count = 0;
    }
}