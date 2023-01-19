
namespace tz
{
	template<std::size_t S>
	stack_allocator<S>::stack_allocator():
	linear_allocator(tz::memblk{.ptr = this->data, .size = S}){}
}
