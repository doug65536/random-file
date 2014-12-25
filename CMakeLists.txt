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

set(sources
    RandomFile/RandomFile.cpp
    RandomFile/stdafx.cpp
    RandomFile/stdafx.h
    RandomFile/targetver.h
    RandomFile/irandomprovider.cpp
    RandomFile/irandomprovider.h
    RandomFile/ifile.cpp
    RandomFile/ifile.h
    RandomFile/utfhelpers.cpp
    RandomFile/utfhelpers.h
)

if(WIN32)
    set(platform_sources
	RandomFile/startup-win32.cpp
	RandomFile/randomprovider-win32.cpp
	RandomFile/randomprovider-win32.h
	RandomFile/file-win32.cpp
	RandomFile/file-win32.h
    )
else()
    set(platform_sources
	RandomFile/startup-cxx.cpp
	RandomFile/randomprovider-urandom.cpp
	RandomFile/randomprovider-urandom.h
	RandomFile/file-cxx.cpp
	RandomFile/file-cxx.h
    )
endif()

add_executable(random-data ${sources} ${platform_sources})
