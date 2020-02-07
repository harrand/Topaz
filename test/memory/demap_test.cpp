//
// Created by Harrand on 24/01/2020.
//

#include "test_framework.hpp"
#include "memory/demap.hpp"
#include <string>

tz::test::Case creation()
{
    tz::test::Case test_case("tz::mem::DeMap Creation Tests");
    tz::mem::DeMap<std::string, int> m;
    using namespace std::literals::string_literals;
    topaz_expect(test_case, !m.contains_key("psidjfslfhj"s), "Empty DeMap wrongly thinks it contains a garbage key");
    topaz_expect(test_case, !m.contains_value(9384759), "Empty DeMap wrongly thinks it contains a garbage value");
    // TODO: Check size etc...
    return test_case;
}

tz::test::Case example_elements()
{
    tz::test::Case test_case("tz::mem::DeMap Forward Element Tests");
    tz::mem::DeMap<std::string, int> m;
    using namespace std::literals::string_literals;
    m.emplace("favourite number"s, 6);
    m.emplace("small number"s, 1);
    m.emplace("big number"s, 862);

    topaz_expect(test_case, m.contains_key("favourite number"s), "DeMap failed to recognise existence of contained key");
    topaz_expect(test_case, m.contains_key("small number"s), "DeMap failed to recognise existence of contained key");
    topaz_expect(test_case, m.contains_key("big number"s), "DeMap failed to recognise existence of contained key");

    topaz_expect(test_case, m.contains_value(6), "DeMap failed to recognise existence of contained value");
    topaz_expect(test_case, m.contains_value(1), "DeMap failed to recognise existence of contained value");
    topaz_expect(test_case, m.contains_value(862), "DeMap failed to recognise existence of contained value");


    {
        auto fav_iter = m.find_by_key("favourite number"s);
        topaz_expect(test_case, fav_iter != m.end(), "DeMap failed to store favourite number properly.");
        topaz_expect(test_case, fav_iter->first == ("favourite number"s), "DeMap failed to store key.");
        topaz_expect(test_case, fav_iter->second == 6, "DeMap failed to store value");
    }
    return test_case;
}

tz::test::Case forward_elements()
{
    tz::test::Case test_case("tz::mem::DeMap Forward Element Tests");
    
    tz::mem::DeMap<std::string, int> m;
    using namespace std::literals::string_literals;
    m.emplace("favourite number"s, 6);
    m.emplace("small number"s, 1);
    m.emplace("big number"s, 862);

    topaz_expect(test_case, m.get_value("favourite number") == 6, "DeMap failed to retrieve value from key properly");
    topaz_expect(test_case, m.get_value("small number") == 1, "DeMap failed to retrieve value from key properly");
    topaz_expect(test_case, m.get_value("big number") == 862, "DeMap failed to retrieve value from key properly");

    return test_case;
}

tz::test::Case backward_elements()
{
    tz::test::Case test_case("tz::mem::DeMap Backward Element Tests");

    tz::mem::DeMap<std::string, int> m;
    using namespace std::literals::string_literals;
    m.emplace("favourite number"s, 6);
    m.emplace("small number"s, 1);
    m.emplace("big number"s, 862);

    topaz_expect(test_case, m.get_key(6) == "favourite number", "DeMap failed to retrieve key from value properly");
    topaz_expect(test_case, m.get_key(1) == "small number", "DeMap failed to retrieve key from value properly");
    topaz_expect(test_case, m.get_key(862) == "big number", "DeMap failed to retrieve key from value properly");

    return test_case;
}

tz::test::Case double_elements()
{
    tz::test::Case test_case("tz::mem::DeMap Double-Ended Element Tests");

    tz::mem::DeMap<std::string, int> m;
    using namespace std::literals::string_literals;
    m.emplace("favourite number"s, 6);
    m.emplace("small number"s, 1);
    m.emplace("big number"s, 862);

    auto val = 6;
    topaz_expect(test_case, m.get_value(m.get_key(val)) == val, "DeMap key+value check failed");

    topaz_expect(test_case, m.contains_key("big number"), "DeMap wrongly doesn't think key exists");
    topaz_expect(test_case, m.contains_value(862), "DeMap wrongly doesn't think value exists");
    m.erase_key("big number");
    topaz_expect(test_case, !m.contains_key("big number"), "DeMap still thinks key exists after erasure");
    topaz_expect(test_case, !m.contains_value(862), "DeMap still thinks value exists after erasure");


    topaz_expect(test_case, m.contains_key("favourite number"), "DeMap wrongly doesn't think key exists");
    topaz_expect(test_case, m.contains_value(6), "DeMap wrongly doesn't think value exists");
    m.erase_value(6);
    topaz_expect(test_case, !m.contains_key("favourite number"), "DeMap still thinks key exists after erasure");
    topaz_expect(test_case, !m.contains_value(6), "DeMap still thinks value exists after erasure");

    topaz_expect(test_case, m.contains_key("small number"), "DeMap wrongly doesn't think key exists");
    topaz_expect(test_case, m.contains_value(1), "DeMap wrongly doesn't think value exists");
    m.set_key(1, "fucking small number");
    topaz_expect(test_case, !m.contains_key("small number"), "DeMap still thinks key exists after erasure");
    topaz_expect(test_case, m.contains_key("fucking small number"), "DeMap wrongly doesn't think key exists");
    topaz_expect(test_case, m.contains_value(1), "DeMap doesn't think value exists after resetting");
    topaz_expect(test_case, m.get_value("fucking small number") == 1, "DeMap could not retrieve key from new value after resetting");

    topaz_expect(test_case, !m.contains_key("alterac"), "DeMap thinks it does (contain the key), but it doesn't");
    m.set_value("alterac", 51);
    topaz_expect(test_case, m.contains_key("alterac"), "DeMap failed to notice emplacement of key via set_value");
    topaz_expect(test_case, m.get_value("alterac") == 51, "DeMap failed to notice emplacement of value via set_value");

    return test_case;
}

int main()
{
	tz::test::Unit dmap;
	
    dmap.add(creation());
    dmap.add(example_elements());
    dmap.add(forward_elements());
    dmap.add(backward_elements());
    dmap.add(double_elements());

	return dmap.result();
}