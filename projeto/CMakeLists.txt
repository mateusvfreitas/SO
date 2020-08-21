cmake_minimum_required(VERSION 3.16)
project(projeto C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(projeto
        datatypes.h
        pingpong.c
        pingpong.h
        queue.c
        queue.h
        pingpong-sleep.c)
