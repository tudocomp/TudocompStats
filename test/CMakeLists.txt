if(NOT GTEST_FOUND)
    MESSAGE(STATUS "gtest is not available - tests disabled!")
    return()
endif()

include(tdc_testsuite)

run_test(tudostats DEPS ${TDC_TEST_DEPS} tudocomp_stat)

