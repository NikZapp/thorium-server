cmake_minimum_required (VERSION 2.6)
project(thorium)

file(GLOB SOURCE_FILES *.cpp)
include_directories(.)

add_executable(thorium ${SOURCE_FILES})