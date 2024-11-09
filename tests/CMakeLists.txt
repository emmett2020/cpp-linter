include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
link_libraries(spdlog git2)

add_executable(test_git test_git.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../src/utils/git_utils.cpp)

