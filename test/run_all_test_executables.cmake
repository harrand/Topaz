# Takes paths to test executables via command line arguments.
# Expect 3rd argument onwards to be test executable paths.
# This script will invoke each test executable, ensuring that all of their return codes are 0.
# If at least one executable doesn't return 0, will indicate a test failure.
function(run_all_tests)
    set(TESTS_SUCCESSFUL 0)
    list(LENGTH "${TEXT_EXECUTABLES}" NUM_EXES)
    message(STATUS "Beginning execution of ${NUM_EXES} tests...")
    message(STATUS "Test Executable Paths: " ${TEST_EXECUTABLES})
    foreach(index RANGE 3 ${CMAKE_ARGC})
        set(executable_path ${CMAKE_ARGV${index}})
        if(NOT ${executable_path} STREQUAL "")
            message(STATUS "Executing test executable \"${executable_path}\"")
            execute_process(COMMAND ${executable_path}
                    RESULT_VARIABLE CUR_RESULT_CODE)
            message(STATUS "Executable \"${executable_path}\" exited with code " ${CUR_RESULT_CODE})
            if(NOT ${CUR_RESULT_CODE} STREQUAL 0)
                message(STATUS "Detected non-zero result code")
                set(TESTS_SUCCESSFUL 1)
            endif()
        endif()
    endforeach()
    if(${TESTS_SUCCESSFUL})
        message(FATAL_ERROR "Detected that at least one test failed!")
    else()
        message(STATUS "Detected that all tests passed!")
    endif()
endfunction()

run_all_tests()