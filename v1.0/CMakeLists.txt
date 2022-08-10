cmake_minimum_required(VERSION 3.0)
project(WebServer)

set(CMAKE_CXX_STANDARD 14)

aux_source_directory(./base base_src)
aux_source_directory(./config config_src)
aux_source_directory(./HTTP HTTP_src)
aux_source_directory(./sql sql_src)
aux_source_directory(./net net_src)

set_target_properties(${TARGET} PROPERTIES
COMPILE_FLAGS "-pthread"
LINK_FLAGS "-pthread")


add_executable(webserver ${base_src} ${config_src} ${HTTP_src} ${sql_src} ${net_src} main.cpp)
target_link_libraries(webserver  ${PROJECT_SOURCE_DIR}/lib/libjsoncpp.a)