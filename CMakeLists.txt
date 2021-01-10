cmake_minimum_required(VERSION 3.16)
project(mysh C)

set(CMAKE_C_STANDARD 99)


## libmyshlauncher ##
add_library(libmyshlauncher STATIC libmyshlauncher.c)

## libmyshparser ##
find_package(BISON)
find_package(FLEX REQUIRED)
BISON_TARGET(bison_parser bison_parser.y ${CMAKE_CURRENT_BINARY_DIR}/bison_parser.c)
FLEX_TARGET(flex_scanner flex_lexer.l ${CMAKE_CURRENT_BINARY_DIR}/flex_lexer.c)
ADD_FLEX_BISON_DEPENDENCY(flex_scanner bison_parser)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library(libmyshparser STATIC
        libmyshparser.c
        ${BISON_bison_parser_OUTPUTS}
        ${FLEX_flex_scanner_OUTPUTS}
        )
target_link_libraries(libmyshparser PUBLIC ${FLEX_LIBRARIES})

## libmyshreader ##
add_library(libmyshreader STATIC libmyshreader.c)
target_link_libraries(libmyshreader -lreadline)

## mysh ##
add_executable(mysh mysh.c)
target_link_libraries(mysh PUBLIC libmyshreader)
target_link_libraries(mysh PUBLIC libmyshparser)
target_link_libraries(mysh PUBLIC libmyshlauncher)
