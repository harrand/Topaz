//
// Created by Harrand on 26/12/2019.
//

#include "memory/block.hpp"
#include "core/debug/assert.hpp"
#include <cstring>

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

	void Block::copy_to(const Block& block)
	{
		topaz_assertf(this->size() <= block.size(), "tz::mem::Block::copy_to(const Block&): Cannot copy source block data (size: %zu bytes) to destination block (size: %zu bytes) because the destination block is too small.", this->size(), block.size());
		std::memcpy(block.begin, this->begin, this->size());
	}

	Block Block::null()
	{
		return {nullptr, nullptr};
	}

	AutoBlock::AutoBlock(std::size_t size): Block(std::malloc(size), size){}

	AutoBlock::AutoBlock(AutoBlock&& move): Block(move.begin, move.end)
	{
		move.begin = nullptr;
		move.end = nullptr;
	}

	AutoBlock::~AutoBlock()
	{
		// Does nothing if this->begin == nullptr.
		std::free(this->begin);
	}
}