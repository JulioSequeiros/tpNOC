cmake_minimum_required(VERSION 4.2)
project(tpNOC C)

set(CMAKE_C_STANDARD 23)

add_executable(tpNOC
        main.c
        module_1.c
        module_2.c
        module_4.c
        module_6.c
        util.c
)