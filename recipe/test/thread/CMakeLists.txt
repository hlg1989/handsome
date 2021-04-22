cmake_minimum_required(VERSION 3.0)
project(thread)

include_directories(../../thread/include)
aux_source_directory(src SOURCES)

add_executable(thread_local_singleton_test thread_local_singleton_test.cpp)
target_link_libraries(thread_local_singleton_test -lpthread)
target_link_libraries(thread_local_singleton_test thread)