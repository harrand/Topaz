//
// Created by Harrand on 21/12/2019.
//

#include "test_framework.hpp"
#include "memory/block.hpp"

TZ_TEST_BEGIN(dist)
	// Here's some data, see if the block knows its own size.
	char data[8];
	tz::mem::Block b1(data, data + 5);
	topaz_expect(b1.size() == 5, "tz::mem::Block had incorrect size. Expected ", 5, " got ", b1.size());
	
	// chars are sizeof 1, so maybe it was getting lucky. size() should return number of bytes, not elements.
	std::uint64_t big_data[8];
	tz::mem::Block b2(big_data, big_data + 5);
	constexpr std::size_t sz = sizeof(std::uint64_t) * 5;
	topaz_expect(b2.size() == sz, "tz::mem::Block had incorrect size for non-char type. Expected ", sz, " got ", b2.size());
TZ_TEST_END

TZ_TEST_BEGIN(auto_block)
	tz::mem::AutoBlock blk{sizeof(int)};
	topaz_expect(blk.size() == sizeof(int), "tz::mem::AutoBlock::size(): Unexpected size. Expected ", sizeof(int), ", got ", blk.size());
TZ_TEST_END

int main()
{
	tz::test::Unit block;
	
	block.add(dist());
	block.add(auto_block());

	return block.result();
}