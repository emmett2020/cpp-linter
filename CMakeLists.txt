cmake_minimum_required(VERSION 3.13)

# Read and watch VERSION file changes
set(VERSION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/VERSION")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${VERSION_FILE})
file(READ "${VERSION_FILE}" CPP_LINTER_VERSION)
string(STRIP "${CPP_LINTER_VERSION}" CPP_LINTER_VERSION)

project(cpp_linter VERSION ${CPP_LINTER_VERSION})

set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED true)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(CMAKE_BUILD_TYPE Debug)
set(linter_root ${CMAKE_CURRENT_SOURCE_DIR})
set(config_dir ${linter_root}/src/configs)

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

configure_file(${config_dir}/version.h.in ${config_dir}/version.h)

FILE(GLOB_RECURSE SRC_FILES "${linter_root}/src/github/*.cpp"
                            "${linter_root}/src/tools/*.cpp"
                            "${linter_root}/src/utils/*.cpp"
                            "${linter_root}/src/*.cpp")

add_executable(cpp-linter ${SRC_FILES})
target_include_directories(cpp-linter PRIVATE ${Boost_INCLUDE_DIRS}
                                              ${magic_enum_SOURCE_DIR}/include
                                              ${linter_root}/src)
target_link_libraries(cpp-linter ${Boost_LIBRARIES}
                                 spdlog
                                 range-v3
                                 httplib
                                 nlohmann_json
                                 tinyxml2
                                 magic_enum
                                 git2)


include(${linter_root}/cmake/FetchCatch2.cmake)
add_subdirectory(tests)
