cmake_minimum_required(VERSION 3.13)
project(linter)

set(CMAKE_CXX_STANDARD 23)
set(CXX_STANDARD_REQUIRED true)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(CMAKE_BUILD_TYPE Debug)

OPTION (ENABLE_COVERAGE "Enable coverage" ON)
IF(ENABLE_COVERAGE)
  message(STATUS ENABLE_COVERAGE=${ENABLE_COVERAGE})
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage ")
  set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} --coverage ")
ENDIF()

# TODO: add an example about how to use boost in CMake
find_package(Boost 1.83.0 REQUIRED)

set(SRC_FILE src/main.cpp)
add_executable(main ${SRC_FILE})
target_include_directories(main PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(main ${Boost_LIBRARIES})
