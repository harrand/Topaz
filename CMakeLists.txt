cmake_minimum_required(VERSION 3.30)
include(cmake/version.cmake)

message(STATUS "Topaz ${TOPAZ_MAJOR_VERSION}.${TOPAZ_MINOR_VERSION}")
project(topaz
	VERSION ${TOPAZ_MAJOR_VERSION}.${TOPAZ_MINOR_VERSION}
	DESCRIPTION "C++20 Graphics Engine"
	HOMEPAGE_URL "https://harrand.github.io/Topaz/"
	LANGUAGES CXX
)

find_package(Threads)
add_subdirectory(cppfront_cmake)