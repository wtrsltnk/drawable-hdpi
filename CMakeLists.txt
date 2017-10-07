cmake_minimum_required(VERSION 3.1)

project(drawable-hdpi)

add_subdirectory(system.io)

add_executable(drawable-hdpi
    program.cpp
    system.io/src/system.io.cpp
    stb/stb_image.h
    stb/stb_image_resize.h
    stb/stb_image_write.h
    )

target_compile_features(drawable-hdpi
    PRIVATE cxx_auto_type
    PRIVATE cxx_nullptr
    PRIVATE cxx_range_for
    )

target_include_directories(drawable-hdpi
    PRIVATE system.io/include
    PRIVATE stb
    )

ADD_CUSTOM_COMMAND(
    TARGET drawable-hdpi
    POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:drawable-hdpi> ../small-bins/
)
