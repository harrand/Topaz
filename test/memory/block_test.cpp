//
// Created by Harrand on 21/12/2019.
//

#include "test_framework.hpp"
#include "memory/block.hpp"

tz::test::Case dist()
{
    tz::test::Case test_case("tz::mem::Block Distance Tests");
    
    // Here's some data, see if the block knows its own size.
    char data[8];
    tz::mem::Block b1(data, data + 5);
    topaz_expect(test_case, b1.size() == 5, "tz::mem::Block had incorrect size. Expected ", 5, " got ", b1.size());
    
    // chars are sizeof 1, so maybe it was getting lucky. size() should return number of bytes, not elements.
    std::uint64_t big_data[8];
    tz::mem::Block b2(big_data, big_data + 5);
    constexpr std::size_t sz = sizeof(std::uint64_t) * 5;
    topaz_expect(test_case, b2.size() == sz, "tz::mem::Block had incorrect size for non-char type. Expected ", sz, " got ", b2.size());

    return test_case;
}

int main()
{
	tz::test::Unit block;
	
    block.add(dist());

	return block.result();
}