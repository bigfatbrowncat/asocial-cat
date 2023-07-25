cmake_minimum_required(VERSION 3.24)

project (asocial-cat)

file (GLOB SOURCE_FILES *.cpp)
file (GLOB HEADER_FILES *.hpp)

add_executable(asocial-cat-server ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(asocial-cat-server ws2_32 wsock32)

target_include_directories(asocial-cat-server PUBLIC
        "${CMAKE_PREFIX_PATH}/include"
        )
