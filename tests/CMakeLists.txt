FILE(GLOB_RECURSE test_files "${linter_root}/tests/*.cpp")
link_libraries(Catch2::Catch2WithMain)
add_executable(test ${test_files} $<TARGET_OBJECTS:dep_obj>)

