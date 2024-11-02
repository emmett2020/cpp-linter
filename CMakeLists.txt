cmake_minimum_required(VERSION 3.13)
project(linter)

set(CMAKE_CXX_STANDARD 23)
set(CXX_STANDARD_REQUIRED true)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(CMAKE_BUILD_TYPE Debug)
set(linter_root ${CMAKE_CURRENT_SOURCE_DIR})

include(${linter_root}/cmake/FetchSpdlog.cmake)
include(${linter_root}/cmake/FetchRangeV3.cmake)

OPTION (ENABLE_COVERAGE "Enable coverage" OFF)
IF(ENABLE_COVERAGE)
  message(STATUS ENABLE_COVERAGE=${ENABLE_COVERAGE})
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage ")
  set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} --coverage ")
ENDIF()

# TODO: add an example about how to use boost in CMake
# TODO: add a startup repository
find_package(Boost 1.83.0 REQUIRED COMPONENTS filesystem system)

set(SRC_FILE
  src/main.cpp
  src/utils/shell.cpp
  src/tools/run_clang_tidy.cpp
)
add_executable(main ${SRC_FILE})
target_include_directories(main PRIVATE ${Boost_INCLUDE_DIRS}
                                        ${linter_root}/src)
target_link_libraries(main ${Boost_LIBRARIES}
                           spdlog
                           range-v3)
