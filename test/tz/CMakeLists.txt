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
add_text(
  TARGET tz_gpu_shader_test
  INPUT_DIR ${PROJECT_SOURCE_DIR}
  OUTPUT_DIR ${PROJECT_BINARY_DIR}
  TEXT_FILES
    test/tz/shaders/empty_compute.spv
    test/tz/shaders/empty_vertex.spv
    test/tz/shaders/empty_fragment.spv
)

topaz_add_test(
  TARGET tz_gpu_compute_renderer_test
  SOURCES
    gpu_compute_renderer_test.cpp
)
add_text(
  TARGET tz_gpu_compute_renderer_test
  INPUT_DIR ${PROJECT_SOURCE_DIR}
  OUTPUT_DIR ${PROJECT_BINARY_DIR}
  TEXT_FILES
    test/tz/shaders/empty_compute.spv
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
add_text(
  TARGET tz_gpu_graphics_renderer_test
  INPUT_DIR ${PROJECT_SOURCE_DIR}
  OUTPUT_DIR ${PROJECT_BINARY_DIR}
  TEXT_FILES
    test/tz/shaders/empty_vertex.spv
    test/tz/shaders/empty_fragment.spv
)