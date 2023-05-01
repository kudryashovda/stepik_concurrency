cmake_minimum_required(VERSION 3.24)
project(libeventServer)

set(CMAKE_CXX_STANDARD 11)

add_executable(${PROJECT_NAME}
        main.cpp)
target_link_libraries(libeventServer event)
