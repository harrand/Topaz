configure_file(${CMAKE_SOURCE_DIR}/version.txt version_txt_dummy.xml)
file(READ ${CMAKE_SOURCE_DIR}version.txt TOPAZ_VERSION_TXT)
string(REGEX MATCH "[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+" TOPAZ_VERSION ${TOPAZ_VERSION_TXT})
