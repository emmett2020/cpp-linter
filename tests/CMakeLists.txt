include_directories(${Boost_INCLUDE_DIRS}
                    ${magic_enum_SOURCE_DIR}/include
                    ${linter_root}/src)
link_libraries(spdlog git2)
link_libraries(${Boost_LIBRARIES}
               spdlog
               range-v3
               httplib
               nlohmann_json
               tinyxml2
               magic_enum
               git2
               Catch2::Catch2WithMain)

set(SRC_DIR "${linter_root}/src")
FILE(GLOB_RECURSE DEP_FILES "${SRC_DIR}/github/*.cpp"
                            "${SRC_DIR}/tools/*.cpp"
                            "${SRC_DIR}/utils/*.cpp"
                            "${SRC_DIR}/progrom_options.cpp"
                            "${SRC_DIR}/context.cpp"
)

add_executable(test_git test_git.cpp ${DEP_FILES})
add_executable(test_program_options test_program_options.cpp ${DEP_FILES})

