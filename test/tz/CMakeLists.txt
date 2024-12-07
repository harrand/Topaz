topaz_add_test(
  TARGET tz_initialise_test
  SOURCES
    initialise.cpp
)

topaz_add_test(
  TARGET tz_bundle_test
  SOURCES
    bundle_test.cpp
)
topaz_bundle_files(
  TARGET tz_bundle_test
  FILES
    files/secret.txt
)

topaz_add_test(
  TARGET tz_io_image_test
  DEFINE_PACKAGE
  SOURCES
    io_image_test.cpp
  BUNDLE_FILES
    files/img.png
)

topaz_add_test(
  TARGET tz_lua_execute_test
  SOURCES
    lua_execute_test.cpp
)

topaz_add_test(
  TARGET tz_vector_test
  SOURCES
    vector_test.cpp
)

topaz_add_test(
  TARGET tz_matrix_test
  SOURCES
    matrix_test.cpp
)

topaz_add_test(
  TARGET tz_gpu_initialise_test
  SOURCES
    gpu_initialise.cpp
)

topaz_add_test(
  TARGET tz_gpu_resource_test
  SOURCES
    gpu_resource_test.cpp
)

topaz_add_test(
  TARGET tz_gpu_shader_test
  SOURCES
    gpu_shader_test.cpp
  SHADERS
    test/tz/shaders/triangle.vertex.tzsl
    test/tz/shaders/triangle.fragment.tzsl
    test/tz/shaders/noop.compute.tzsl
)

topaz_add_test(
  TARGET tz_gpu_compute_renderer_test
  SOURCES
    gpu_compute_renderer_test.cpp
  SHADERS
    test/tz/shaders/noop.compute.tzsl
)

topaz_add_test(
  TARGET tz_gpu_graphics_renderer_test
  SOURCES
    gpu_graphics_renderer_test.cpp
  SHADERS
    test/tz/shaders/triangle.vertex.tzsl
    test/tz/shaders/triangle.fragment.tzsl
)

topaz_add_test(
  TARGET tz_ren_quad_test
  SOURCES
    ren_quad_test.cpp
)