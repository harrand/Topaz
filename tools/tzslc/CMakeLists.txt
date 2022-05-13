add_tool(
	TARGET tzslc
	SOURCE_FILES
		preprocessor.cpp
		preprocessor.hpp
		source_transform.hpp
		stdlib.hpp
		tzslc_main.cpp
	TEXT_FILES
		matrix.tzsl
		space.tzsl
)
