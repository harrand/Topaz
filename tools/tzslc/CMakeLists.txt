add_tool(
	TARGET tzslc
	SOURCE_FILES
		header_export.cpp
		header_export.hpp
		preprocessor.cpp
		preprocessor.hpp
		source_transform.hpp
		stdlib.hpp
		tzslc_main.cpp
)

add_text(
	TARGET tzslc
	TEXT_FILES
		tools/tzslc/matrix.tzsl
		tools/tzslc/space.tzsl
)
