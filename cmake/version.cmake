set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/version.txt)
file(READ ${CMAKE_SOURCE_DIR}/version.txt TOPAZ_VERSION_TXT)

# Search for a version (MAJOR.MINOR) within a string.
# e.g parse_version(my_major my_minor "application version: 123.45")
# my_major = 123
# my_minor = 45
function(parse_version out_major out_minor large_string)
	string(REGEX MATCH "[0-9]+\.[0-9]+" version_string ${large_string})
	string(REPLACE "." ";" version_list ${version_string})
	list(GET version_list 0 _major)
	list(GET version_list 1 _minor)
	set(${out_major} ${_major} PARENT_SCOPE)
	set(${out_minor} ${_minor} PARENT_SCOPE)
endfunction()
parse_version(TOPAZ_MAJOR_VERSION TOPAZ_MINOR_VERSION ${TOPAZ_VERSION_TXT})
