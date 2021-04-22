cmake_minimum_required(VERSION 3.0)
project(thread)

include_directories(${HANDSOME_ROOT_DIRECTORY}/recipe/thread/include)

set(THREAD_TEST_NAME thread_local_singleton_test)
add_executable(${THREAD_TEST_NAME} thread_local_singleton_test.cpp)
target_link_libraries(${THREAD_TEST_NAME} -lpthread)
target_link_libraries(${THREAD_TEST_NAME} thread)