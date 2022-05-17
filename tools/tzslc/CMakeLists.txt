add_tool(
	TARGET tzslc
	SOURCE_FILES
		source_transform.hpp
		stdlib.hpp
		tzsl.cpp
		tzsl.hpp
		tzslc_main.cpp
	TEXT_FILES
		shaders/matrix.tzsl
		shaders/space.tzsl
)
