cmake_minimum_required(VERSION 3.0.2)
project(TudocompStats)

set(CMAKE_CXX_STANDARD 11)

if(STATS_DISABLED)
    message("[CAUTION] Statistic tracking is disabled!")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DSTATS_DISABLED")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DSTATS_DISABLED")
endif(STATS_DISABLED)

file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(tudocomp_stats STATIC ${SOURCE_FILES})
target_include_directories(tudocomp_stats PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
