cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)
project(TudocompStats)

# Check if this project is build standalone
#
# We do this in case we want to use this repo as a GIT submodule,
# because then we only need the source files themselves
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(STANDALONE 1)
endif()

if(STANDALONE)
    # init build system
    execute_process(COMMAND git submodule update --init -- build_system
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/submodules)

    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/submodules/build_system/cmakemodules")
    include(tdc_init)

    # downloadable dependencies
    include(depend_glog)

    # quit if dependencies aren't met
    tdc_check_hard_deps()
    if(TDC_DEPS_MISSING)
        return()
    endif()

    # soft dependencies
    include(softdepend_gtest)

    # submodules
    include(git_submodule_subdirectories)
    git_submodule_subdirectory(submodules/build_system)
endif()

# Disable stat tracking altogether
if(STATS_DISABLED)
    message("[CAUTION] Statistic tracking is disabled!")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DSTATS_DISABLED")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DSTATS_DISABLED")
endif(STATS_DISABLED)

# Disable only malloc counter
if(MALLOC_DISABLED)
    message("[CAUTION] malloc override is disabled, no memory tracking possible!")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DMALLOC_DISABLED")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DMALLOC_DISABLED")
endif(MALLOC_DISABLED)

# Main target
if(DYNAMIC)
    add_library(tudocomp_stat SHARED
        src/tudocomp_stat/malloc.cpp
        src/tudocomp_stat/StatPhase.cpp
    )
else()
    add_library(tudocomp_stat STATIC
        src/tudocomp_stat/malloc.cpp
        src/tudocomp_stat/StatPhase.cpp
    )
endif()

target_include_directories(tudocomp_stat PUBLIC include)

if(STANDALONE)
    # Unit tests
    add_subdirectory(test)

    # Disclaimer
    MESSAGE(STATUS "Built Type: " ${CMAKE_BUILD_TYPE} )
endif()

