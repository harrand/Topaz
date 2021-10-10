#include <cstring>
namespace tz::gl
{
	template<typename T>
	BufferData BufferData::from_array(std::span<const T> data)
	{
		BufferData buf;
		buf.data.resize(data.size_bytes());
		std::memcpy(buf.data.data(), data.data(), data.size_bytes());
		return buf;
	}

	template<typename T>
	BufferData BufferData::from_value(const T& data)
	{
		BufferData buf;
		buf.data.resize(sizeof(T));
		std::memcpy(buf.data.data(), &data, sizeof(T));
		return buf;
	}
}