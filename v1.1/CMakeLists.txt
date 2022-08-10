cmake_minimum_required(VERSION 3.0)

project(WebServer)

set(CXX_FLAGS
    -g
    -Wall
    -std=c++14
    -pthread
    -Wno-unused-parameter
    -lmysqlclient
)

set(SRC 
    
    )
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

include_directories(${PROJECT_SOURCE_DIR}/json)
aux_source_directory(. src)

add_executable(server ${src})
target_link_libraries(server jsoncpp)