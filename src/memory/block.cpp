//
// Created by Harrand on 26/12/2019.
//

#include "memory/block.hpp"

namespace tz::mem
{
	std::size_t byte_distance(void* a, void* b)
	{
		std::ptrdiff_t signed_distance = std::distance(reinterpret_cast<char*>(a), reinterpret_cast<char*>(b));
		return static_cast<std::size_t>(std::abs(signed_distance));
	}

	Block::Block(void* begin, void* end): begin(begin), end(end){}

	Block::Block(void* begin, std::size_t size): Block(begin, reinterpret_cast<void*>(reinterpret_cast<char*>(begin) + size)){}

	std::size_t Block::size() const
	{
		return byte_distance(this->begin, this->end);
	}

	Block Block::null()
	{
		return {nullptr, nullptr};
	}

	AutoBlock::AutoBlock(std::size_t size): Block(std::malloc(size), size){}

	AutoBlock::~AutoBlock()
	{
		std::free(this->begin);
	}
}