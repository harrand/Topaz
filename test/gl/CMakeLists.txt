if(${TOPAZ_OGL})
	add_subdirectory(ogl)
	add_subdirectory(ogl2)
elseif(${TOPAZ_VULKAN})
	add_subdirectory(vk2)
endif()

add_tz_test(NAME tz_resource_test
	SOURCE_FILES tz_resource_test.cpp
	GRAPHICAL
	)
