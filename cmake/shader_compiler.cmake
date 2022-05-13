include(CMakeParseArguments)

macro(add_shader_vulkan)
	# example: C:\VulkanSDK\1.2.170.0\Bin\glslc.exe
	set(VK_BIN_DIR $ENV{VULKAN_SDK}/Bin)
	set(VK_GLSLC_EXECUTABLE_PATH ${VK_BIN_DIR}/glslc)

	set(shader_path ${PROJECT_SOURCE_DIR}/${SHADER})
	set(processed_shader_name ${SHADER}.glsl)
	set(processed_shader_path ${PROJECT_BINARY_DIR}/${processed_shader_name})

	add_custom_command(
		OUTPUT ${processed_shader_path}
		COMMENT "TZSLC_VK: Preprocessing ${SHADER} -> ${processed_shader_name}"
		COMMAND "${TZSLC_EXECUTABLE_PATH}" ${shader_path} -mall -o ${processed_shader_path}
		DEPENDS tzslc ${shader_path}
		IMPLICIT_DEPENDS CXX ${shader_path}
		VERBATIM
	)
	set(spv_name ${SHADER}.spv)
	set(spv_path ${PROJECT_BINARY_DIR}/${spv_name})

	add_custom_command(
		OUTPUT ${spv_path}
		COMMENT "TZSLC_VK: Building ${processed_shader_name} -> ${spv_name}"
		COMMAND ${VK_GLSLC_EXECUTABLE_PATH} -o ${spv_path} ${processed_shader_path}
		DEPENDS tzslc ${processed_shader_path}
		IMPLICIT_DEPENDS CXX ${processed_shader_path}
		VERBATIM)

	set(output_path ${spv_path})
	set_source_files_properties(${processed_shader_path} ${output_path} PROPERTIES GENERATED TRUE)
endmacro()

macro(add_shader_opengl)
	set(shader_path ${PROJECT_SOURCE_DIR}/${SHADER})
	set(output_name ${SHADER}.glsl)
	set(output_path ${PROJECT_BINARY_DIR}/${output_name})

	add_custom_command(
		OUTPUT ${output_path}
		COMMENT "TZSLC_OGL: Building ${SHADER} -> ${output_name}"
		COMMAND "${TZSLC_EXECUTABLE_PATH}" ${shader_path} -mall -o ${output_path}
		DEPENDS tzslc ${shader_path}
		IMPLICIT_DEPENDS CXX ${processed_shader_path}
		VERBATIM
	)

	set_source_files_properties(${output_path} PROPERTIES GENERATED TRUE)
endmacro()

function(add_shader)
	set(TZSLC_EXECUTABLE_PATH $<TARGET_FILE:tzslc>)
	cmake_parse_arguments(
		ADD_SHADER
		""
		"TARGET"
		"SHADERS"
		${ARGN}
	)

	foreach(SHADER ${ADD_SHADER_SHADERS})
		if(${TOPAZ_RENDERAPI} MATCHES "opengl")
			add_shader_opengl()
		elseif(${TOPAZ_RENDERAPI} MATCHES "vulkan")
			add_shader_vulkan()
		endif()
		# `output_path` is the path to the built shader (i.e glsl (ogl) or spv (vk)).
		STRING(REGEX REPLACE "^${CMAKE_SOURCE_DIR}/" "" offset_dir ${output_path})
		message(WARNING "built shader: ${offset_dir}")
		add_text(
			TARGET ${ADD_SHADER_TARGET}
			INPUT_DIR ${PROJECT_SOURCE_DIR}
			OUTPUT_DIR ${PROJECT_SOURCE_DIR}
			TEXT_FILES ${offset_dir}
		)

	endforeach()
endfunction()
