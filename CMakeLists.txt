cmake_minimum_required(VERSION 3.2.0)

if ($ENV{TRAVIS_BUILD_NUMBER})
    project(ptyqt VERSION 0.0.$ENV{TRAVIS_BUILD_NUMBER} LANGUAGES CXX)
else ($ENV{TRAVIS_BUILD_NUMBER})
    project(ptyqt VERSION 0.0.1 LANGUAGES CXX)
endif ($ENV{TRAVIS_BUILD_NUMBER})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

#params
#available params:
# - NO_BUILD_TESTS=1
# - NO_BUILD_EXAMPLES=1
# - WINPTY_ROOT_DIR=<path/to/winpty> - if you set this parameter, winpty doesn't be auto-downloaded like dependency
IF("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(PTYQT_DEBUG TRUE)
    add_definitions(-DPTYQT_DEBUG)
else()
    set(PTYQT_DEBUG FALSE)
endif()

if (MSVC)
    #target arch for find winpty libs
    if ("${TARGET_ARCH}" STREQUAL "x86_amd64")
        set(TARGET_ARCH x64)
    endif()
    if ("${TARGET_ARCH}" STREQUAL "")
        set(TARGET_ARCH x64) #x86 / x64
    endif()
    set(BUILD_TYPE STATIC) #STATIC / SHARED
    set(WINPTY_ROOT_DIR "") #user can define his own installation of WinPty for prevent auto-downloading dependency

    #message("TARGET_ARCH ${TARGET_ARCH}")
endif()

#dependencies
add_subdirectory(thirdparty)
find_package(Qt5Core REQUIRED)

if (MSVC)
    find_package(Qt5Network REQUIRED)
endif()

if (MSVC)
    message("WinPty root dir " ${WINPTY_ROOT_DIR})
    message("WinPty libs " ${WINPTY_LIBS})
endif()

#install
#set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})

set(PTYQT_INSTALL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(PTYQT_INSTALL_BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
set(PTYQT_INSTALL_LIB_DIR ${PROJECT_SOURCE_DIR}/lib)

set(PTYQTLIB_HEADERS_DIR ${PROJECT_SOURCE_DIR}/core)

include_directories(${PTYQT_INSTALL_INCLUDE_DIR})
include_directories(${PTYQTLIB_HEADERS_DIR})
if (MSVC)
    include_directories(${WINPTY_ROOT_DIR}/include)
endif()

enable_testing()

#sub projects
add_subdirectory(core)
if (NOT "${NO_BUILD_TESTS}" STREQUAL "1")
  add_subdirectory(tests)
endif()
if (NOT "${NO_BUILD_EXAMPLES}" STREQUAL "1")
    add_subdirectory(examples)
endif()
