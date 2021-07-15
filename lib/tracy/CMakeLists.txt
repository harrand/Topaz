cmake_minimum_required(VERSION 3.10)
# Tracy Client Library (Based on tracy.pdf from May 19, 2021)
add_library(TracyClient
    TracyClient.cpp
)
target_include_directories(TracyClient PUBLIC ./)
target_compile_options(TracyClient PUBLIC -DTRACY_ENABLE)
target_link_libraries(TracyClient PUBLIC ws2_32 imagehlp dbghelp)

# Tracy Server Executable (Based on same tracy.pdf)
# Requires installation of various dependencies on each platform.
# We will do a local installation within the project directory - No other changes to the machine are made.
if(WIN32)
    # There is a magic script we can run.
    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        add_custom_target(TracyServerDepInstall
            COMMENT "Installing Tracy Server dependencies via vcpkg (MSVC)"
            COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/install_vcpkg_dependencies.bat)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_custom_target(TracyServerDepInstall
        COMMENT "Installing Tracy Server dependencies via vcpkg (GNU/Clang)"
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/install_vcpkg_dependencies_mingw.bat)
    endif()
elseif(UNIX)
    # We require pkg-config.
else()
    message(STATUS "Warning: Tracy Server is not supported for this platform")
endif()

# TODO: Create TracyServer target.
add_executable(TracyServer
common/TracySocket.cpp
common/TracyStackFrames.cpp
common/TracySystem.cpp
common/tracy_lz4.cpp
common/tracy_lz4hc.cpp
imgui/imgui.cpp
imgui/imgui_demo.cpp
imgui/imgui_draw.cpp
imgui/imgui_tables.cpp
imgui/imgui_widgets.cpp
imgui/misc/freetype/imgui_freetype.cpp
nfd/nfd_common.c
nfd/nfd_win.cpp
server/TracyBadVersion.cpp
server/TracyColor.cpp
server/TracyFilesystem.cpp
server/TracyMemory.cpp
server/TracyMicroArchitecture.cpp
server/TracyMmap.cpp
server/TracyMouse.cpp
server/TracyPrint.cpp
server/TracySourceContents.cpp
server/TracySourceTokenizer.cpp
server/TracySourceView.cpp
server/TracyStorage.cpp
server/TracyTaskDispatch.cpp
server/TracyTexture.cpp
server/TracyTextureCompression.cpp
server/TracyThreadCompress.cpp
server/TracyUserData.cpp
server/TracyView.cpp
server/TracyWeb.cpp
server/TracyWorker.cpp
zstd/common/debug.c
zstd/common/entropy_common.c
zstd/common/error_private.c
zstd/common/fse_decompress.c
zstd/common/pool.c
zstd/common/threading.c
zstd/common/xxhash.c
zstd/common/zstd_common.c
zstd/compress/fse_compress.c
zstd/compress/hist.c
zstd/compress/huf_compress.c
zstd/compress/zstdmt_compress.c
zstd/compress/zstd_compress.c
zstd/compress/zstd_compress_literals.c
zstd/compress/zstd_compress_sequences.c
zstd/compress/zstd_compress_superblock.c
zstd/compress/zstd_double_fast.c
zstd/compress/zstd_fast.c
zstd/compress/zstd_lazy.c
zstd/compress/zstd_ldm.c
zstd/compress/zstd_opt.c
zstd/decompress/huf_decompress.c
zstd/decompress/zstd_ddict.c
zstd/decompress/zstd_decompress.c
zstd/decompress/zstd_decompress_block.c
zstd/dictBuilder/cover.c
zstd/dictBuilder/divsufsort.c
zstd/dictBuilder/fastcover.c
zstd/dictBuilder/zdict.c
profiler/libs/gl3w/GL/gl3w.c
profiler/src/HttpRequest.cpp
profiler/src/imgui_impl_glfw.cpp
profiler/src/imgui_impl_opengl3.cpp
profiler/src/main.cpp
profiler/src/NativeWindow.cpp
profiler/src/ResolvService.cpp
profiler/src/winmain.cpp
profiler/src/winmainArchDiscovery
profiler/build/win32/Tracy.rc
)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(TracyServerLinkDirectories ${PROJECT_BINARY_DIR}/lib/tracy/vcpkg/installed/x64-windows-static/lib)
    set(TracyServerIncludeDirectories ${PROJECT_BINARY_DIR}/lib/tracy/vcpkg/installed/x64-windows-static/include ${PROJECT_BINARY_DIR}/lib/tracy/vcpkg/installed/x64-windows-static/include/capstone)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(TracyServerLinkDirectories ${PROJECT_BINARY_DIR}/lib/tracy/vcpkg/installed/x64-mingw-static/lib)
    set(TracyServerIncludeDirectories ${PROJECT_BINARY_DIR}/lib/tracy/vcpkg/installed/x64-mingw-static/include ${PROJECT_BINARY_DIR}/lib/tracy/vcpkg/installed/x64-mingw-static/include/capstone)
endif()
target_include_directories(TracyServer PRIVATE
profiler/libs/gl3w
imgui
${TracyServerIncludeDirectories}
)
add_dependencies(TracyServer TracyServerDepInstall)
target_link_directories(TracyServer PRIVATE ${TracyServerLinkDirectories})
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_link_libraries(TracyServer PRIVATE brotlicommon-static brotlidec-static brotlienc-static ws2_32 freetype glfw3 png16 zlib bz2 capstone)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_link_libraries(TracyServer PRIVATE ws2_32 freetype glfw3 png16 zlib bz2 capstone oleaut32 imm32)

endif()
target_compile_options(TracyServer PRIVATE -mlzcnt)
target_compile_definitions(TracyServer PRIVATE -DNOMINMAX)