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

add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE stb)