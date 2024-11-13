add_subdirectory(concurrentqueue)
add_subdirectory(VulkanMemoryAllocator)
add_subdirectory(textc)

add_library(lua STATIC
	lua/lapi.c
	lua/lcode.c
	lua/lctype.c
	lua/ldebug.c
	lua/ldo.c
	lua/ldump.c
	lua/lfunc.c
	lua/lgc.c
	lua/llex.c
	lua/lmem.c
	lua/lobject.c
	lua/lopcodes.c
	lua/lparser.c
	lua/lstate.c
	lua/lstring.c
	lua/ltable.c
	lua/ltm.c
	lua/lundump.c
	lua/lvm.c
	lua/lzio.c
	lua/lauxlib.c
	lua/lbaselib.c
	lua/lcorolib.c
	lua/ldblib.c
	lua/liolib.c
	lua/lmathlib.c
	lua/loadlib.c
	lua/loslib.c
	lua/lstrlib.c
	lua/ltablib.c
	lua/lutf8lib.c
	lua/linit.c
	)
target_include_directories(lua PUBLIC "${PROJECT_SOURCE_DIR}/lib/lua")

add_library(imgui STATIC
	imgui/imgui.cpp
	imgui/imgui_demo.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_tables.cpp
	imgui/imgui_widgets.cpp

	imgui/misc/cpp/imgui_stdlib.cpp
)
target_include_directories(imgui PUBLIC "${PROJECT_SOURCE_DIR}/lib/imgui")
target_compile_definitions(imgui PRIVATE "-DIMGUI_USER_CONFIG=\"${PROJECT_SOURCE_DIR}/lib/topaz_imgui_config.hpp\"")

add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE stb)