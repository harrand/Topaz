# Custom CMakeLists written by Harrand. GLAD from https://glad.dav1d.de/. Details:
# OpenGL Core Profile (4.6)
# One extension: GL_ARB_bindless_texture

cmake_minimum_required(VERSION 3.9)

add_library(glad src/glad.c)
target_include_directories(glad PUBLIC ./include)