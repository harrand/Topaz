cmake_minimum_required(VERSION 3.9)

add_library(vma INTERFACE)
target_include_directories(vma INTERFACE ./include)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
target_compile_options(vma INTERFACE -Wno-nullability-extension -Wno-nullability-completeness -Wno-nested-anon-types -Wno-missing-field-initializers -Wno-unused-parameter -Wno-unused-variable)
endif()