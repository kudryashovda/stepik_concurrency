cmake_minimum_required(VERSION 3.24)
project(libev_echo_server)

set(CMAKE_CXX_STANDARD 11)

add_executable(${PROJECT_NAME}
        main.cpp)
target_link_libraries(${PROJECT_NAME} ev)
