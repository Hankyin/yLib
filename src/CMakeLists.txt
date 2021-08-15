cmake_minimum_required(VERSION 3.5)
project(ylib)

aux_source_directory(./core CORE_SRC)
aux_source_directory(./database DB_SRC)
aux_source_directory(./database/sqlite DB_SQLITE_SRC)
aux_source_directory(./network NET_SRC)
aux_source_directory(./utils UTILS_SRC)
aux_source_directory(./utils/minizip UTILS_ZIP_SRC)

set(ALL_SRC ${CORE_SRC} 
        ${DB_SRC} 
        ${DB_SQLITE_SRC} 
        ${NET_SRC}
        ${UTILS_SRC}
        ${UTILS_ZIP_SRC})

add_library(${PROJECT_NAME} STATIC ${ALL_SRC})


set(CMAKE_CXX_STANDARD 11) # 指定C++版本
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR})
# target_link_libraries(${PROJECT_NAME} PRIVATE pthread)