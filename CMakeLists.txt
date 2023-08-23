cmake_minimum_required(VERSION 3.15)

project(muteteams)

add_executable(
            muteteams
            WIN32 # no prompt
            muteteams.cpp
            icon.rc
        )
add_executable(
            muteteams_bg
            WIN32 # no prompt
            muteteams_bg.cpp
            icon.rc
        )
