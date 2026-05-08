cmake_minimum_required(VERSION 4.2)
project(tpNOC C)

set(CMAKE_C_STANDARD 23)

add_executable(tpNOC main.c
        noc.h)
