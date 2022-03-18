
namespace tz
{
	template<std::size_t S>
	StackAllocator<S>::StackAllocator():
	LinearAllocator(tz::Blk{.ptr = this->data, .size = S}){}
}
