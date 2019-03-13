# Pty-Qt - C++ library for work with PseudoTerminals

Ubuntu 18.04: [![Build Status](https://travis-ci.com/kafeg/ptyqt.svg?token=8hsFoXZ5FooS7bytYzXZ&branch=master)](https://travis-ci.com/kafeg/ptyqt)

Pty-Qt is small library for access to console applications by pseudo-terminal interface on Mac, Linux and Windows. On Mac and Linux you can use standard PseudoTerminal API and on Windows you can use WinPty or ConPty.

## Pre-Requirements
  - ConPty part works only on Windows 10 >= 1903 (build > 18309) and can be built only with Windows SDK >= 10.0.18346.0 (maybe >= 17134, but not sure)
  - WinPty part requires winpty sdk for build and winpty.dll with winpty-agent.exe for deployment with target application. WinPty can work on Windows XP and later (depended on used build SDK: vc140 / vc140_xp). You can't link WinPty libraries inside your App, because it use cygwin for build.
  - UnixPty part can work on both Linux/Mac versions, because it based on standard POSIX pseudo terminals API
  - Еarget platforms: x86 or x64
  - Required Qt >= 5.10

## Build on Windows
```sh
git clone https://github.com/Microsoft/vcpkg.git vcpkg
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg integrate install
./vcpkg install qt-base:x64-windows qt5-network:x64-windows
git clone https://github.com/kafeg/ptyqt.git ptyqt
mkdir ptyqt-build
cd ptyqt-build
<VCPKG_ROOT>/downloads/tools/cmake-3.12.4-windows/cmake-3.12.4-win32-x86/bin/cmake.exe ../ptyqt "-DCMAKE_TOOLCHAIN_FILE=<VCPKG_ROOT>/scripts/buildsystems/vcpkg.cmake" "-DVCPKG_TARGET_TRIPLET=x64-windows"
<VCPKG_ROOT>/downloads/tools/cmake-3.12.4-windows/cmake-3.12.4-win32-x86/bin/cmake.exe --build . --target winpty
<VCPKG_ROOT>/downloads/tools/cmake-3.12.4-windows/cmake-3.12.4-win32-x86/bin/cmake.exe --build .
```

## Build on Ubuntu
```sh
sudo apt-get install qtbase5-dev cmake libqt5websockets5-dev
git clone https://github.com/kafeg/ptyqt.git ptyqt
mkdir ptyqt-build
cd ptyqt-build
cmake ../ptyqt
cmake --build .
```

## Build on Mac
Install Qt and Qt Creator. Open project and build it

## Usage
For use this library, just build it, link with 'pyqt' library and include 'ptyqt.h' into your project

## Run tests
...

## Run examples
1. xtermjs
- build and run example
- install nodejs (your preffer way)
- open console and run:
```sh
cd ptyqt/examples/xtermjs
npm install xterm
npm install http-server -g
http-server ./
```
- open http://127.0.0.1:8080/ in Web browser

## More information
This library based on:
  - https://github.com/Microsoft/node-pty
  - https://github.com/Microsoft/console
  - https://github.com/rprichard/winpty
  - https://github.com/xtermjs/xterm.js
  - https://github.com/lxqt/qterminal
