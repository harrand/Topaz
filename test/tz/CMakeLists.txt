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