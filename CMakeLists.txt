cmake_minimum_required(VERSION 3.7)
project(chat C)

set(CMAKE_C_STANDARD 99)

add_executable(chat main.c client.c server.c errors.c errors.h)
target_link_libraries(chat pthread)

