cmake_minimum_required(VERSION 3.5)
project(ylibtest)

aux_source_directory(./ TEST_SRC)


set(ALL_SRC ${TEST_SRC} )
 
add_executable(${PROJECT_NAME} ${ALL_SRC})

set(CMAKE_CXX_STANDARD 11) # 指定C++版本
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR})
# target_link_libraries(${PROJECT_NAME} PRIVATE pthread)