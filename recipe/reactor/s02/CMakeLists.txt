cmake_minimum_required(VERSION 3.0)
project(reactor-s02)


include_directories(${HANDSOME_ROOT_DIRECTORY}/recipe/thread/include)
include_directories(${HANDSOME_ROOT_DIRECTORY}/recipe/logging/include)
include_directories(${HANDSOME_ROOT_DIRECTORY}/recipe/datetime/include)
aux_source_directory(. SOURCES)

add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_link_libraries(${PROJECT_NAME} -lpthread)
target_link_libraries(${PROJECT_NAME} thread logging datetime)
