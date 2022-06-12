#include "core/callback.hpp"
#include "core/assert.hpp"

void increment_free_function(int& x)
{
	x++;
}

int main()
{
	int counter = 0;

	auto increment_lambda = [](int& x){x++;};

	// Empty callback shouldnt do anything
	tz::Callback<int&> cb;
	cb(counter);
	tz_assert(counter == 0, "Callback logic failed (empty callback). Expected %d, got %d", 0, counter);

	// Add free function to increment, should go to 1
	auto h1 = cb.add_callback(increment_free_function);
	cb(counter);
	tz_assert(counter == 1, "Callback logic failed (registered callable not working). Expected %d, got %d", 1, counter);

	// Remove free function so callback is empty again, shouldn't do anything.
	cb.remove_callback(h1);
	cb(counter);
	tz_assert(counter == 1, "Callback logic failed (unregistered callable still working?). Expected %d, got %d", 1, counter);

	auto h2 = cb.add_callback(increment_lambda);
	cb(counter);
	tz_assert(counter == 2, "Callback logic failed (registered callable not working). Expected %d, got %d", 2, counter);

	h1 = cb.add_callback(increment_free_function);
	cb(counter);
	tz_assert(counter == 4, "Callback logic failed (registered callables not working). Expected %d, got %d", 4, counter);

	return 0;
}