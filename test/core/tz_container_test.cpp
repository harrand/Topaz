#include "tz/core/data/free_list.hpp"

struct nullable_int
{
	static nullable_int null()
	{
		return {.i = std::numeric_limits<int>::max()};
	}

	int i;

	bool is_null() const
	{
		return this->i == std::numeric_limits<int>::max();
	}
	bool operator==(const nullable_int& rhs) const = default;
};

struct nullable_class
{
public:
	nullable_class(int x, float y, char z):
	x(x), y(y), z(z){}
	bool is_null() const
	{
		return this->x == std::numeric_limits<int>::max() && this->y == 0.0f && this->z == '\0';
	}

	static nullable_class null()
	{
		return {std::numeric_limits<int>::max(), 0.0f, '\0'};
	}
private:
	int x;
	float y;
	char z;
};

void free_list()
{
	// push back, size, empty, clear
	tz::free_list<nullable_int> ints;
	tz::assert(ints.size() == 0);
	tz::assert(ints.empty());
	ints.push_back({.i = 5});
	tz::assert(ints.size() == 1);
	tz::assert(!ints.empty());
	ints.clear();
	tz::assert(ints.size() == 0);
	tz::assert(ints.empty());

	// emplace back
	tz::free_list<nullable_class> classes;
	tz::assert(classes.size() == 0);
	tz::assert(classes.empty());
	classes.push_back(nullable_class{5, 6.9f, 'e'});
	tz::assert(classes.size() == 1);
	classes.emplace_back(5, 6.9f, 'e');
	tz::assert(classes.size() == 2);

	// iterators
	ints = {};	
	// fibonacci
	// 0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610
	int a = 0;
	int b = 1;
	for(std::size_t i = 0; i < 16; i++)
	{
		int fib = a + b;
		a = b;
		b = fib;
		ints.push_back({.i = fib});
	}
	std::vector<int> fibs{1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597};
	std::size_t internal_counter = 0;
	// easy case: no gaps in the array.
	for(nullable_int ni : ints)
	{
		tz::assert(!ni.is_null());
		tz::assert(ni.i == fibs[internal_counter], "id %zu - %d must equal %d", internal_counter, ni.i, fibs[internal_counter]);
		internal_counter++;
	}

	nullable_int magic = {.i = 89}; // this is the 9th fibonacci number.
	tz::assert(ints.size() == 16);
	// make sure a certain element exists.
	auto iter = std::find(ints.begin(), ints.end(), magic);
	tz::assert(iter != ints.end(), "list of fibonacci numbers did not contain %d", magic.i);
	// now erase it.
	ints.erase(static_cast<tz::hanval>(9));
	tz::assert(ints.size() == 15);
	// should no longer exist.
	iter = std::find(ints.begin(), ints.end(), magic);
	tz::assert(iter == ints.end(), "free_list was not able to properly remove %d, or there is a bug in its iterator.", magic.i);
	// now do the iteration again.
	internal_counter = 0;
	for(nullable_int ni : ints)
	{
		// there should be no nulls still
		// this is because iterator++ should automatically skip over nulls.
		tz::assert(!ni.is_null());
		internal_counter++;
	}
	tz::assert(internal_counter == 15);

	// adding a new integer again should replace the old thing (which was element 9 - 89)
	// its now 99999.
	auto handle = ints.push_back({.i = 99999});
	tz::assert(static_cast<std::size_t>(static_cast<tz::hanval>(handle)) == 9u);
	tz::assert(ints[handle].i == 99999);
	tz::assert(ints.size() == 16);
	// write into the actual fibonacci array.
	// both the arrays should both be completely equal again.
	fibs[9] = 99999;
	internal_counter = 0;
	for(nullable_int ni : ints)
	{
		tz::assert(!ni.is_null());
		tz::assert(ni.i == fibs[internal_counter], "id %zu - %d must equal %d", internal_counter, ni.i, fibs[internal_counter]);
		internal_counter++;
	}
}

int main()
{
	free_list();
}
