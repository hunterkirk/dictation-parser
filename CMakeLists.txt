cmake_minimum_required(VERSION 3.20)

project(dictation-parser
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(dictation-parser
    src/main.cpp
    src/DictationParser.cpp
)

target_include_directories(dictation-parser
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if (MSVC)
    target_compile_options(dictation-parser PRIVATE /W4)
else()
    target_compile_options(dictation-parser PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# Optimized release builds.
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Build type" FORCE)
endif()
