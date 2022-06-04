add_tool(
	TARGET tzslc
	SOURCE_FILES
		assert.hpp
		source_transform.hpp
		stdlib.hpp
		tzsl.cpp
		tzsl.hpp
		tzslc_main.cpp
	TEXT_FILES
		shaders/atomic.tzsl
		shaders/math.tzsl
		shaders/matrix.tzsl
		shaders/space.tzsl
)
