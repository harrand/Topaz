topaz_add_test(
  TARGET tz_initialise_test
  SOURCES
    initialise.cpp
)

topaz_add_test(
  TARGET tz_vector_test
  SOURCES
    vector_test.cpp
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
)
add_shader(
  TARGET tz_gpu_shader_test
  SHADERS
    test/tz/shaders/triangle.vertex.tzsl
    test/tz/shaders/triangle.fragment.tzsl
    test/tz/shaders/noop.compute.tzsl
)

topaz_add_test(
  TARGET tz_gpu_compute_renderer_test
  SOURCES
    gpu_compute_renderer_test.cpp
)
add_shader(
  TARGET tz_gpu_compute_renderer_test
  SHADERS
    test/tz/shaders/noop.compute.tzsl
)

topaz_add_test(
  TARGET tz_gpu_graphics_renderer_test
  SOURCES
    gpu_graphics_renderer_test.cpp
)
add_shader(
  TARGET tz_gpu_graphics_renderer_test
  SHADERS
    test/tz/shaders/triangle.vertex.tzsl
    test/tz/shaders/triangle.fragment.tzsl
)