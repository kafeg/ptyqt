cmake_minimum_required(VERSION 3.2.0)

project(ptyqt VERSION 0.1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

#params
set(PTYQT_DEBUG TRUE)
#set(PTYQT_DEBUG FALSE)
if ("${PTYQT_DEBUG}")
    add_definitions(-DPTYQT_DEBUG)
endif()

if (MSVC)
    set(TARGET_ARCH x64) #x86 / x64
    set(BUILD_TYPE STATIC) #STATIC / SHARED
    set(WINPTY_ROOT_DIR "") #user can define his own installation of WinPty for prevent auto-downloading dependency
endif()

#dependencies
add_subdirectory(thirdparty)
find_package(Qt5Core REQUIRED)
if (MSVC)
    find_package(Qt5Network REQUIRED)
endif()

message("WinPty root dir " ${WINPTY_ROOT_DIR})
message("WinPty libs " ${WINPTY_LIBS})

#install
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})

set(PTYQT_INSTALL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(PTYQT_INSTALL_BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
set(PTYQT_INSTALL_LIB_DIR ${PROJECT_SOURCE_DIR}/lib)

set(PTYQTLIB_HEADERS_DIR ${PROJECT_SOURCE_DIR}/core)

include_directories(${PTYQT_INSTALL_INCLUDE_DIR})
include_directories(${PTYQTLIB_HEADERS_DIR})
include_directories(${WINPTY_ROOT_DIR}/include)

#sub projects
add_subdirectory(core)
add_subdirectory(tests)
#add_subdirectory(examples)

#add_executable(${PROJECT_NAME} "main.cpp")

#target_link_libraries(${PROJECT_NAME} Qt5::Core)
