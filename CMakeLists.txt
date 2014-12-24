cmake_minimum_required(VERSION 2.8)
if(MSVC)
	add_definitions(-DUNICODE -D_UNICODE)
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_executable(random-data${CMAKE_EXECUTABLE_SUFFIX}
    RandomFile/RandomFile.cpp
    RandomFile/stdafx.cpp
    RandomFile/stdafx.h
    RandomFile/targetver.h
)

