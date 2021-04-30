cmake_minimum_required(VERSION 3.0)
project(reactor-s06-test)


include_directories(${HANDSOME_ROOT_DIRECTORY}/recipe/thread/include)
include_directories(${HANDSOME_ROOT_DIRECTORY}/recipe/logging/include)
include_directories(${HANDSOME_ROOT_DIRECTORY}/recipe/datetime/include)
include_directories(${HANDSOME_ROOT_DIRECTORY}/recipe/reactor/s06)
aux_source_directory(. SOURCES)

add_executable(${PROJECT_NAME} s06-test8.cpp)

target_link_libraries(${PROJECT_NAME} reactor-s06)