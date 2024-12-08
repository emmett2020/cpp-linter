cmake_minimum_required(VERSION 3.13)
project(cpp-linter)

set(CMAKE_CXX_STANDARD 23)
set(CXX_STANDARD_REQUIRED true)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(CMAKE_BUILD_TYPE Debug)
set(linter_root ${CMAKE_CURRENT_SOURCE_DIR})

include(${linter_root}/cmake/FetchSpdlog.cmake)
include(${linter_root}/cmake/FetchRangeV3.cmake)
include(${linter_root}/cmake/FetchHttpLib.cmake)
include(${linter_root}/cmake/FetchNlohmannJson.cmake)
include(${linter_root}/cmake/FetchMagicEnum.cmake)
include(${linter_root}/cmake/FetchTinyXML2.cmake)

OPTION (ENABLE_COVERAGE "Enable coverage" OFF)
IF(ENABLE_COVERAGE)
  message(STATUS ENABLE_COVERAGE=${ENABLE_COVERAGE})
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage ")
  set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} --coverage ")
ENDIF()

find_package(Boost 1.83.0 REQUIRED COMPONENTS filesystem system regex program_options)

set(SRC_FILE
  src/main.cpp
  src/utils/shell.cpp
  src/utils/program_options.cpp
  src/utils/env_manager.cpp
  src/utils/git_utils.cpp
  src/utils/context.cpp
  src/tools/clang_tidy.cpp
  src/tools/clang_format.cpp
  src/github/api.cpp
  src/github/common.cpp
)

add_executable(cpp-linter ${SRC_FILE})
target_include_directories(cpp-linter PRIVATE ${Boost_INCLUDE_DIRS}
                                              ${linter_root}/src)
target_link_libraries(cpp-linter ${Boost_LIBRARIES}
                                 spdlog
                                 range-v3
                                 httplib
                                 nlohmann_json
                                 tinyxml2
                                 git2)


include(${linter_root}/cmake/FetchCatch2.cmake)
add_subdirectory(tests)
