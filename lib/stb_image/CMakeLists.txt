cmake_minimum_required(VERSION 3.9)

add_library(stbi
    stb_image.c
    stb_image_write.c)
target_include_directories(stbi PUBLIC ./)