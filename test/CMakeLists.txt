cmake_minimum_required(VERSION 3.5)
project(ylib_test)

aux_source_directory(. TEST_SRC)
set(ALL_SRC  ${TEST_SRC})


set(CMAKE_CXX_STANDARD 17) # 指定C++版本,
add_executable(${PROJECT_NAME} ${ALL_SRC})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} PRIVATE ylib)
target_link_libraries(${PROJECT_NAME} PRIVATE pthread)
