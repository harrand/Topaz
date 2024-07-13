cmake_minimum_required(VERSION 3.30)
include(cmake/version.cmake)

project(topaz VERSION ${TZ_VERSION}
	DESCRIPTION "C++20 Graphics Engine")

find_package(Threads)
add_subdirectory(cppfront_cmake)