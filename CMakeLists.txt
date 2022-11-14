add_subdirectory(vendor/HDK)
project(topaz)

hdk_add_library(TARGET topaz
	SOURCES
		src/tz/tz.hpp
		src/tz/tz.cpp
)

target_include_directories(topaz PUBLIC ${PROJECT_SOURCE_DIR}/src)
