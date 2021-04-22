cmake_minimum_required(VERSION 3.0)
project(logging)

include_directories(include)
include_directories(..)
aux_source_directory(src SOURCES)

add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_link_libraries(${PROJECT_NAME} -lpthread)