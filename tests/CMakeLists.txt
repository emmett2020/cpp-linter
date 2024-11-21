set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src)
set(UTILS_DIR ${SRC_DIR}/utils)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
link_libraries(spdlog git2 Catch2::Catch2WithMain)

add_executable(test_git test_git.cpp ${UTILS_DIR}/git_utils.cpp)

