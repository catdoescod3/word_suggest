cmake_minimum_required(VERSION 4.0)

project(word_suggest)

set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS Off)
set(CMAKE_EXPORT_COMPILE_COMMANDS On)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/trie.cpp
    src/suffix_engine.cpp
)
