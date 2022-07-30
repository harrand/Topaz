#ifndef TOPAZ_CORE_IMPORTED_TEXT_HPP
#define TOPAZ_CORE_IMPORTED_TEXT_HPP

#define TZ_MACROHACKERY_STRINGIFY(X) TZ_MACROHACKERY_STRINGIFY2(X)    
#define TZ_MACROHACKERY_STRINGIFY2(X) #X
#define TZ_MACROHACKERY_CONCAT(X, Y) X##Y
#define TZ_MACROHACKERY_JOIN3(X, Y, Z) X##Y##Z
#define TZ_MACROHACKERY_JOIN4(X, Y, Z, W) X##Y##Z##W

/**
 * @ingroup tz_core
 * @fn ImportedTextHeader(text_name, text_ext)
 * @hideinitializer
 * Retrieves a file path which is intended to be #included in a application's main source file. Once included, the imported text's contents can be retrieved as a constexpr string_view via @ref ImportedTextData.
 */
#define ImportedTextHeader(text_name, text_ext) TZ_MACROHACKERY_STRINGIFY2(text_name.text_ext.hpp)

/**
 * @ingroup tz_core
 * @fn ImportedTextData(text_name, text_ext)
 * @hideinitializer
 * Retrieves a token representing a `std::string_view` which contains an imported text file's contents. You can only retrieve an imported text's contents if the imported text has been included via @ref ImportedTextHeader
 */
#define ImportedTextData(text_name, text_ext) []()->std::string_view{std::span<const std::byte> shader_bin = std::as_bytes(std::span<const std::int8_t>(TZ_MACROHACKERY_JOIN3(text_name, _, text_ext))); return std::string_view{reinterpret_cast<const char*>(shader_bin.data()), shader_bin.size_bytes()};}()

#endif // TOPAZ_CORE_IMPORTED_TEXT_HPP
