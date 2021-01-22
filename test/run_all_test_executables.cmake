# Takes paths to test executables via command line arguments.
# Expect 3rd argument onwards to be test executable paths.
# This script will invoke each test executable, ensuring that all of their return codes are positive. If a test executable succeeded, it will return a positive value representing the number of test cases. If a test failed, it will return a negative number.
# If at least one executable has a negative return value, will indicate a test failure.
function(run_all_tests)
    set(TESTS_SUCCESSFUL 0)
    math(EXPR NUM_UNIT_TESTS "${CMAKE_ARGC} - 3")
    set(NUM_PASSING_TEST_CASES 0)
    message(STATUS "Beginning execution of " ${NUM_UNIT_TESTS} " unit-test executables...")
    foreach(index RANGE 3 ${CMAKE_ARGC})
        set(executable_path ${CMAKE_ARGV${index}})
        if(NOT ${executable_path} STREQUAL "")
            execute_process(COMMAND ${executable_path}
                    RESULT_VARIABLE CUR_RESULT_CODE OUTPUT_QUIET)
            message(STATUS "Executable \"${executable_path}\" exited with code " ${CUR_RESULT_CODE})
            if(${CUR_RESULT_CODE} GREATER_EQUAL 0)
                math(EXPR NUM_PASSING_TEST_CASES "${NUM_PASSING_TEST_CASES} + ${CUR_RESULT_CODE}")
                math(EXPR TESTS_SUCCESSFUL "${TESTS_SUCCESSFUL} + 1")
            else()
                message(STATUS "Detected negative result code")
            endif()
        endif()
    endforeach()
    if(NOT ${TESTS_SUCCESSFUL} EQUAL ${NUM_UNIT_TESTS})
        math(EXPR NUM_FAILED_UNIT_TESTS "${NUM_UNIT_TESTS} - ${TESTS_SUCCESSFUL}")
        message(FATAL_ERROR "Detected that ${NUM_FAILED_UNIT_TESTS} test(s) failed!")
    else()
        message(STATUS "Detected that all unit tests passed (${TESTS_SUCCESSFUL}/${NUM_UNIT_TESTS} unit test sources, ${NUM_PASSING_TEST_CASES}/${NUM_PASSING_TEST_CASES}) total cases")
    endif()
endfunction()

run_all_tests()