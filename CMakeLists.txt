cmake_minimum_required(VERSION 3.1)

project(drawable-hdpi)

add_executable(drawable-hdpi
    program.cpp
    )

target_compile_features(drawable-hdpi
    PRIVATE cxx_auto_type
    PRIVATE cxx_nullptr
    PRIVATE cxx_range_for
    )
