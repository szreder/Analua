project(luaparse)
cmake_minimum_required(VERSION 2.8)

find_package(BISON)
find_package(FLEX)

set(CMAKE_CXX_FLAGS "-Wall -std=c++17 -ggdb -fsanitize=address,undefined")

bison_target(Parser grammar.yy ${CMAKE_CURRENT_BINARY_DIR}/Parser.cpp)
flex_target(Lexer scanner.ll ${CMAKE_CURRENT_BINARY_DIR}/Lexer.cpp)
add_flex_bison_dependency(Lexer Parser)

include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/bin")

set (SRC_FILES
	Driver.cpp
	main.cpp
)

add_executable(luaparse ${FLEX_Lexer_OUTPUTS} ${BISON_Parser_OUTPUTS} ${SRC_FILES})
