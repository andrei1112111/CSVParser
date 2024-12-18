cmake_minimum_required(VERSION 3.30)
project(CSVParser)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(./)

add_library(CSVParser_Lib SHARED CSVParser.h)
set_target_properties(CSVParser_Lib PROPERTIES LINKER_LANGUAGE CXX)


add_executable(CSVParser main.cpp)
target_link_libraries(CSVParser PRIVATE CSVParser_Lib)


#add_executable(CSVParser subt1.cpp)
