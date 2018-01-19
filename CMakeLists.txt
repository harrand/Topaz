cmake_minimum_required(VERSION 3.9)
project(topaz VERSION 0.6.0 DESCRIPTION "Cross-Platform Multimedia Engine")
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 17)
include_directories(topaz ${PROJECT_SOURCE_DIR}/inc)
include_directories(topaz ${PROJECT_SOURCE_DIR}/src)
#file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
link_directories(${PROJECT_SOURCE_DIR}/lib)
add_library(topaz SHARED 
src/glew.c
src/audio.cpp
src/boundary.cpp
src/camera.cpp
src/command.cpp
src/data.cpp
src/engine.cpp
src/entity.cpp
src/graphics.cpp
src/gui.cpp
src/gui_display.cpp
src/gui_widget.cpp
src/listener.cpp
src/matrix.cpp
src/mesh.cpp
src/object.cpp
src/physics.cpp
src/quaternion.cpp
src/scene.cpp
src/shader.cpp
src/texture.cpp
src/time.cpp
)
target_link_libraries(topaz OpenGL32 SDL2 SDL2_mixer SDL2_ttf SDL2main mdl)