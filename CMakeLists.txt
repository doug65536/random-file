cmake_minimum_required(VERSION 2.8)
if(MSVC)
	add_definitions(-DUNICODE -D_UNICODE)
endif()

if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64 -municode")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m64 -municode")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -m64 -municode")
    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -m64 -municode")
endif()

if(WIN32)
    # Define WINDOWS_UTF16 on windows
    add_definitions(-DWINDOWS_UTF16)
endif()

# Configure for linux/windows specific code
include(CheckIncludeFiles)
check_include_files(windows.h HAVE_WINDOWS_H)
check_include_files(tchar.h HAVE_TCHAR_H)
check_include_files(unistd.h HAVE_UNISTD_H)

configure_file(RandomFile/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/generated/config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(random-data
    RandomFile/RandomFile.cpp
    RandomFile/stdafx.cpp
    RandomFile/stdafx.h
    RandomFile/targetver.h
)

