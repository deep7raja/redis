cmake_minimum_required(VERSION 4.0)
project(redis)

add_compile_options(-Wall -Wextra -Wpedantic -g)

add_executable(server src/server.cpp src/util.cpp)
add_executable(client src/client.cpp)
