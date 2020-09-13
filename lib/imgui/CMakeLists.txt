cmake_minimum_required(VERSION 3.9)

add_library(imgui
    imconfig.h
    imgui_draw.cpp
    imgui_demo.cpp
    imgui_internal.h
    imgui_widgets.cpp
    imgui.cpp
    imgui.h
    imstb_rectpack.h
    imstb_textedit.h
    imstb_truetype.h)
target_include_directories(imgui PUBLIC ./)