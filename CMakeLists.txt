cmake_minimum_required(VERSION 3.2.0)

project(ptyqt VERSION 0.6.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

#params
#available params:
# - NO_BUILD_TESTS=1
# - NO_BUILD_EXAMPLES=1
IF("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(PTYQT_DEBUG TRUE)
    add_definitions(-DPTYQT_DEBUG)
else()
    set(PTYQT_DEBUG FALSE)
endif()

if("${BUILD_TYPE}" STREQUAL "SHARED")
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

message("Build type: ${BUILD_TYPE}")

if (MSVC)
    #target arch for find winpty libs
    if ("${TARGET_ARCH}" STREQUAL "x86_amd64" OR "${TARGET_ARCH}" STREQUAL "")
        set(TARGET_ARCH x64) # x64
    endif()
	if ("${VCPKG_TARGET_TRIPLET}" MATCHES "x86")
        set(TARGET_ARCH x86) #x86 
    endif()
endif()

#dependencies
find_package(Qt5Core REQUIRED)

if (MSVC)
    find_library(WINPTY_LIBRARIES NAMES winpty)
    find_package(Qt5Network REQUIRED)
	
	message("WINPTY_LIBRARIES ${WINPTY_LIBRARIES}")

    if ("${WINPTY_LIBRARIES}" STREQUAL "")
        message(FATAL "WinPty libs not set!")
	endif()
endif()

#install
set(PTYQT_INSTALL_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include)
set(PTYQT_INSTALL_BIN_DIR ${CMAKE_INSTALL_PREFIX}/bin)
set(PTYQT_INSTALL_LIB_DIR ${CMAKE_INSTALL_PREFIX}/lib)

set(PTYQTLIB_HEADERS_DIR ${PROJECT_SOURCE_DIR}/core)

include_directories(${PTYQT_INSTALL_INCLUDE_DIR})
include_directories(${PTYQTLIB_HEADERS_DIR})
if (MSVC)
    include_directories(${WINPTY_ROOT_DIR}/include)
endif()

#sub projects
add_subdirectory(core)

if (NOT "${NO_BUILD_TESTS}" STREQUAL "1")
  enable_testing()
  add_subdirectory(tests)
endif()

if (NOT "${NO_BUILD_EXAMPLES}" STREQUAL "1")
    add_subdirectory(examples)
endif()
