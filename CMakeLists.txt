cmake_minimum_required(VERSION 3.24)

project (asocial-cat)

file (GLOB SOURCE_FILES *.cpp)
file (GLOB HEADER_FILES *.hpp)

add_subdirectory(third_party/pugixml)
add_subdirectory(third_party/rapidjson)

add_executable(asocial-cat-server ${SOURCE_FILES} ${HEADER_FILES})
add_dependencies(asocial-cat-server pugixml::static)
target_link_libraries(asocial-cat-server ws2_32 wsock32 pugixml::static crypto)
target_include_directories(asocial-cat-server PUBLIC
        "${CMAKE_PREFIX_PATH}/include"
        third_party/pugixml/src
        third_party/rapidjson/include
)

