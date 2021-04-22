cmake_minimum_required(VERSION 3.0)
project(datetime)

include_directories(include)
include_directories(${HANDSOME_ROOT_DIRECTORY}/recipe/)
aux_source_directory(src SOURCES)

add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_link_libraries(${PROJECT_NAME} -lpthread)